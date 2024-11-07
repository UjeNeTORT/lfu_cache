#ifndef CACHE_HPP
#define CACHE_HPP

#include <iostream>
#include <list>
#include <limits>
#include <map>
#include <unordered_map>
#include <list>

namespace caches {

constexpr size_t DefaultCapacity = 0;

template <typename KeyT, typename T>
class LFU_cache {

  class FrqNode;
  class ValNode;

  using FrqNodeIt = std::list<FrqNode>::iterator;
  using ValNodeIt = std::list<ValNode>::iterator;
  using DataAccessFunc = T (*)(const KeyT &);

  size_t hits_ = 0;
  size_t size_ = 0;
  size_t capacity_;

  std::unordered_map<KeyT, ValNodeIt> map_;
  std::list<FrqNode> frq_nodes_;

public:
  explicit LFU_cache(size_t capacity = DefaultCapacity) :
    capacity_(std::max(capacity, DefaultCapacity)), map_(capacity) {}

  template <DataAccessFunc AccessData>
  const T &get(const KeyT &key) {
    auto map_it = map_.find(key);

    if (map_it == map_.end())
      map_it = add<AccessData>(key);
    else
      hits_++;

    ValNodeIt val = inc_freq(map_it->second);
    // inc_freq makes map_it invalid
    map_it = map_.find(val->key());

    return map_it->second->data();
  }

  template <DataAccessFunc AccessData>
  auto add(const KeyT &key) {
    return add(key, AccessData(key));
  }

  size_t freq(const KeyT &key) const {
    auto map_it = map_.find(key);
    if (map_it == map_.end()) return 0;
    return map_it->second->freq();
  }

  size_t hits() const { return hits_; }
  size_t size() const { return size_; }
  size_t capacity() const { return capacity_; }

  void dump(std::ostream &out) {
    out << "================= LFU dump =================\n";
    out << "hits_     = " << hits_ << '\n';
    out << "size_     = " << size_ << '\n';
    out << "capacity_ = " << capacity_ << '\n';

    out << "MAP:\n";
    for (auto el : map_)
      out << '\t' << el.first << " -> " << el.second->data() << '\n';

    out << "FREQ NODES:\n";
    for (auto frq : frq_nodes_) frq.dump(out);
    out << "============================================\n";
  }

protected:
  auto add(const KeyT &key, const T &data) {
    auto map_it = map_.find(key);
    if (map_it != map_.end()) return map_it;

    if (size_ == capacity_) displace();

    FrqNodeIt first_frq = frq_nodes_.begin();

    if (first_frq == frq_nodes_.end() || first_frq->freq() != 0) {
      frq_nodes_.emplace(first_frq, FrqNode{});
      first_frq = frq_nodes_.begin();
    }

    ValNodeIt val = first_frq->add_val(ValNode{data, key, first_frq});

    map_it = map_.emplace(key, val).first;
    size_++;

    return map_it;
  }

private:
  ValNodeIt inc_freq(ValNodeIt val) {
    FrqNodeIt frq = val->frq;
    FrqNodeIt next_frq = std::next(frq);

    if (next_frq == frq_nodes_.end() || next_frq->freq() != frq->freq() + 1)
      next_frq = frq_nodes_.insert(next_frq, FrqNode{frq->freq() + 1});

    // this may be bad
    ValNodeIt new_val = next_frq->add_val(*val, next_frq);
    frq->rm_val(val);
    if (frq->size() == 0) frq_nodes_.erase(frq);

    // upd map_
    map_.erase(new_val->key());
    map_.emplace(new_val->key(), new_val);
    return new_val;
  }

  void displace() {
    if (size_ == 0) {
      std::cerr << "Can't displace from empty cache\n";
      return;
    }

    // todo
    const KeyT &displaced_key = frq_nodes_.front().displace();
    map_.erase(displaced_key);
    size_--;
  }

  class FrqNode {
    size_t freq_;
    std::list<ValNode> val_nodes_;

  public:
    FrqNode(size_t freq = 0) : freq_(freq) {}

    ValNodeIt add_val(ValNode &val, FrqNodeIt frq) {
      val.frq = frq;
      return val_nodes_.insert(val_nodes_.begin(), val);
    }

    //? i dont understand this rvalue ref
    ValNodeIt add_val(ValNode &&val) {
      return val_nodes_.insert(val_nodes_.begin(), val);
    }

    void rm_val(ValNodeIt val) {
      val_nodes_.erase(val);
    }

    KeyT displace() {
      KeyT displaced_key = val_nodes_.back().key();
      val_nodes_.pop_back();
      return displaced_key;
    }

    size_t freq() const { return freq_; }
    size_t size() const { return val_nodes_.size(); }

    void dump(std::ostream &out) {
      out << "\tfreq = " << freq_ << ": ";
      for (auto val : val_nodes_) out << val.key() << ' ';
      out << '\n';
    }
  };

  class ValNode {
    T data_;
    KeyT key_;

  public:
    FrqNodeIt frq;

    ValNode(const T &data, const KeyT &key, FrqNodeIt frq) :
                       data_(data), key_(key), frq(frq) {}

    const T &data()   const { return data_; }
    const KeyT &key() const { return key_; }
    size_t freq()     const { return frq->freq(); }
  };
};

template <typename KeyT, typename T>
class Belady_cache {
  using DataAccessFunc = T (*)(const KeyT &);

  size_t hits_ = 0;
  size_t size_ = 0;
  size_t capacity_;

  std::deque<KeyT> query_;
  std::unordered_map<KeyT, T> map_;

  DataAccessFunc AccessData_;

public:
  explicit
  Belady_cache(size_t capacity, std::deque<T> query, DataAccessFunc AccessData) :
    capacity_(std::max(capacity, DefaultCapacity)), query_(query),
    map_(capacity_), AccessData_(AccessData) {}

  size_t hits() const { return hits_; }
  size_t size() const { return size_; }
  size_t capacity() const { return capacity_; }

private:
  const T &get(const KeyT &key) {
    if (key != query_.front())
      std::cerr << "Expected and received key mismatch\n";

    query_.pop_front();

    auto map_it = map_.find(key);
    if (map_it == map_.end()) map_it = add(key);
    else hits_++;

    return map_it->second;
  }

  auto add(const KeyT &key) {
    auto map_it = map_.find(key);
    if (map_it != map_.end()) return map_it;

    if (size_ == capacity_) displace();

    map_it = map_.emplace(key, AccessData_(key)).first;
    size_++;

    return map_it;
  }

  void displace() {
    const KeyT &displace_key = displace_choose();
    map_.erase(displace_key);
    size_--;
  }

  KeyT displace_choose() {
    std::unordered_map<KeyT, int> next_query;

    int i = 0;
    for (auto it = query_.begin(); it != query_.end(); ++it, ++i) {
      // if elem not in map - skip
      // as we can displace only existing elems
      if (map_.find(*it) == map_.end()) continue;

      // add not tracked elem's first occurence
      if (next_query.find(*it) == next_query.end()) next_query.emplace(*it, i);
    }

    // key in cache which wont be accessed anymore
    for (auto map_it = map_.begin(); map_it != map_.end(); ++map_it) {
      if (next_query.find(map_it->first) == next_query.end())
        return map_it->first;
    }

    auto displace = next_query.begin();
    for (auto it = next_query.begin(); it != next_query.end(); ++it)
      if (it->second > next_query.find(displace->first)->second)
        displace = it;

    return displace->first;
  }

public:
  void dump(std::ostream &out) {
    out << "============== Belady dump =================\n";
    out << "cache: ";
    for (auto &p: map_) out << p.first << " ";
    out << '\n';
    out << "queue: ";
    for (auto &q : query_) out << q << " ";
    out << '\n';
    out << "============================================\n";
  }

  void run() {
    int n_queries = query_.size();
    for (int i = 0; i < n_queries; i++) {
      // //for debug
      // std::cerr << "get(" << query_.front() << ") = "
                // << get(query_.front()) << "\n";
      // dump(std::cout);
      get(query_.front());
    }
  }
};

} // namespace caches

#endif // CACHE_HPP
