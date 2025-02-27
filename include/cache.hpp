#ifndef CACHE_HPP
#define CACHE_HPP

#include <iostream>
#include <limits>
#include <list>
#include <map>
#include <numeric>
#include <unordered_map>

namespace caches {

constexpr size_t DefaultCapacity = 0;

template <typename KeyT, typename T>
class LFU {

  class FrqNode;
  class ValNode;

  using FrqNodeIt = typename std::list<FrqNode>::iterator;
  using ValNodeIt = typename std::list<ValNode>::iterator;
  using DataAccessFunc = T (*)(const KeyT &);

  size_t hits_ = 0;
  size_t size_ = 0;
  size_t capacity_ = DefaultCapacity;

  std::unordered_map<KeyT, ValNodeIt> cache_;
  std::list<FrqNode> frq_nodes_;

  DataAccessFunc AccessData_;

public:
  LFU() : cache_(capacity_), frq_nodes_(capacity_) {}

  explicit
  LFU(size_t capacity, DataAccessFunc AccessData) :
    capacity_(capacity), cache_(capacity), AccessData_(AccessData) {}

  const T &get(const KeyT &key) {
    auto cache_it = cache_.find(key);

    if (cache_it == cache_.end())
      cache_it = add(key);
    else
      hits_++;

    ValNodeIt val = inc_freq(cache_it->second);
    // inc_freq makes map_it invalid
    cache_it = cache_.find(val->key());

    return cache_it->second->data();
  }

  auto add(const KeyT &key) {
    return add(key, AccessData_(key));
  }

  size_t freq(const KeyT &key) const {
    auto cache_it = cache_.find(key);
    if (cache_it == cache_.end()) return 0;
    return cache_it->second->freq();
  }

  size_t hits() const { return hits_; }
  size_t size() const { return size_; }
  size_t capacity() const { return capacity_; }

  void dump(std::ostream &out) const {
    out << "================= LFU dump =================\n";
    out << "hits_     = " << hits_ << '\n';
    out << "size_     = " << size_ << '\n';
    out << "capacity_ = " << capacity_ << '\n';

    out << "MAP:\n";
    for (auto el : cache_)
      out << '\t' << el.first << " -> " << el.second->data() << '\n';

    out << "FREQ NODES:\n";
    for (auto frq : frq_nodes_) frq.dump(out);
    out << "============================================\n";
  }

protected:
  auto add(const KeyT &key, const T &data) {
    auto cache_it = cache_.find(key);
    if (cache_it != cache_.end()) return cache_it;

    if (size_ == capacity_) displace();

    FrqNodeIt first_frq = frq_nodes_.begin();

    if (first_frq == frq_nodes_.end() || first_frq->freq() != 0) {
      frq_nodes_.emplace(first_frq, FrqNode{});
      first_frq = frq_nodes_.begin();
    }

    ValNodeIt val = first_frq->add_val(ValNode{data, key, first_frq});

    cache_it = cache_.emplace(key, val).first;
    size_++;

    return cache_it;
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
    if (frq->empty()) frq_nodes_.erase(frq);

    // upd cache_
    cache_.erase(new_val->key());
    cache_.emplace(new_val->key(), new_val);
    return new_val;
  }

  void displace() {
    if (size_ == 0) {
      std::cerr << "Can't displace from empty cache\n";
      return;
    }

    const KeyT &displaced_key = frq_nodes_.front().displace();
    cache_.erase(displaced_key);
    size_--;
  }

  class FrqNode {
    size_t freq_;
    std::list<ValNode> val_nodes_;

  public:
    FrqNode(size_t freq = 0) : freq_(freq) {}

    ValNodeIt add_val(ValNode &val, FrqNodeIt frq) {
      val.frq = frq;
      val_nodes_.push_front(val);
      return val_nodes_.begin();
    }

    ValNodeIt add_val(const ValNode &val) {
      val_nodes_.push_front(val);
      return val_nodes_.begin();
    }

    void rm_val(ValNodeIt val) {
      val_nodes_.erase(val);
    }

    KeyT displace() {
      KeyT displaced_key = val_nodes_.back().key();
      val_nodes_.pop_back();
      return displaced_key;
    }

    bool empty() const { return val_nodes_.size() == 0; }
    size_t freq() const { return freq_; }
    size_t size() const { return val_nodes_.size(); }

    void dump(std::ostream &out) const {
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
class Belady {
  using DataAccessFunc = T (*)(const KeyT &);

  size_t hits_ = 0;
  size_t size_ = 0;
  size_t capacity_ = DefaultCapacity;

  // In order to predict displace keys, here each list is responsible for
  // occurence of one key in query_.
  // Each elem of a list is a number of queries until get(this key) query.
  // This is done to quickly choose most remote key element for displace
  //
  // use dump() to examine next_query_ structure
  std::unordered_map<KeyT, std::list<int>> next_query_;
  // list representing current state of further queries
  std::list<KeyT> query_;

  size_t curr_n_query_ = 0;

  // "cached" values
  std::unordered_map<KeyT, T> cache_;

  DataAccessFunc AccessData_;

public:
  Belady() : query_(), cache_(capacity_) {};
  explicit
  Belady(size_t capacity, std::list<KeyT> query, DataAccessFunc AccessData) :
    capacity_(capacity), query_(query), cache_(capacity), AccessData_(AccessData) {

    int n_query = 0;
    for (auto const &q : query_) {
      auto cache_it = next_query_.find(q);
      if (cache_it == next_query_.end())
        cache_it = next_query_.emplace(q, std::list<int>{}).first;

      cache_it->second.push_back(n_query);
      n_query++;
    }
  }

  bool empty() const { return size_ == 0; }
  size_t hits() const { return hits_; }
  size_t size() const { return size_; }
  size_t capacity() const { return capacity_; }

private:
  T get(const KeyT &key) {
    auto cache_it = cache_.find(key);
    if (cache_it == cache_.end()) {
      // do not store value which will never be accessed again
      if (next_query_.find(key)->second.size() == 1) {
        T res_val = AccessData_(key);
        update_queries();
        return res_val;
      }

      cache_it = add(key);
    } else {
      hits_++;
    }

    update_queries();

    return cache_it->second;
  }

  auto add(const KeyT &key) {
    auto cache_it = cache_.find(key);
    if (cache_it != cache_.end()) return cache_it;

    if (size_ == capacity_) displace();

    cache_it = cache_.emplace(key, AccessData_(key)).first;
    size_++;

    return cache_it;
  }

  void displace() {
    KeyT displace_candidate = cache_.begin()->first;

    // former displace_choose()
    auto curr_max = cache_.begin();
    auto curr_next_query = next_query_.begin();
    auto curr_next_max = next_query_.find(curr_max->first);

    for (auto cache_it = cache_.begin(), cache_end = cache_.end();
                                            cache_it != cache_end; ++cache_it) {
      curr_next_query = next_query_.find(cache_it->first);

      if (curr_next_query->second.empty()) {
        curr_max = cache_it;
        curr_next_max = next_query_.find(curr_max->first);
        break;
      }

      if (curr_next_query->second.front() >
                        curr_next_max->second.front()) {
        curr_max = cache_it;
        curr_next_max = next_query_.find(curr_max->first);
      }
    }

    displace_candidate = curr_max->first;

    cache_.erase(displace_candidate);
    size_--;
  }

  void update_queries() {
    auto curr_q = next_query_.find(query_.front());
    if (curr_q != next_query_.end()) curr_q->second.pop_front();
    else std::cerr << "ERROR: wrong next_query_ order!\n";

    query_.pop_front();
  }

public:
  void dump(std::ostream &out) const {
    out << "============== Belady dump =================\n";
    out << "size/capacity: " << size_ << "/" << capacity_ << '\n';
    out << "hits = " << hits_ << '\n';
    out << "cache: ";
    for (auto &p: cache_) out << p.first << " ";
    out << '\n';
    out << "queue: ";
    for (auto &q : query_) out << q << " ";
    out << '\n';
    out << "next_query:\n";
    for (auto cache_it = next_query_.begin(); cache_it != next_query_.end(); ++cache_it) {
      out << '\t' << cache_it->first << ": ";
      for (auto &q_delta : cache_it->second) out << q_delta << ' ';
      out << '\n';
    }
    out << "============================================\n";
  }

  void run() {
    int n_queries = query_.size();
    for (int i = 0; i < n_queries; i++) {
      // if (i % 1000 == 0) std::cout << i << '\n';
      get(query_.front());
    }
  }
};

} // namespace caches

template <typename KeyT, typename T>
std::ostream& operator<<(std::ostream& out, const caches::LFU<KeyT, T>& lfu) {
  lfu.dump(out);
  return out;
}

template <typename KeyT, typename T>
std::ostream& operator<<(std::ostream& out, const caches::Belady<KeyT, T>& belady) {
  belady.dump(out);
  return out;
}

#endif // CACHE_HPP
