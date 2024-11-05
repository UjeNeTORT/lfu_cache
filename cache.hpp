#include <iostream>
#include <unordered_map>
#include <list>

namespace caches {

const size_t DEFAULT_CAPACITY = 100;

template <typename KeyT, typename T>
class LFU_cache {

  class FrqNode;
  class ValNode;

  using FrqNodeIt = std::list<FrqNode>::iterator;
  using ValNodeIt = std::list<ValNode>::iterator;
  using DataAccessFunc = T (*)(KeyT);

  size_t hits_ = 0;
  size_t size_ = 0;
  size_t capacity_ = DEFAULT_CAPACITY;

  std::unordered_map<KeyT, ValNodeIt> map_;
  std::list<FrqNode> frq_nodes_;

public:
  LFU_cache(size_t capacity = DEFAULT_CAPACITY) :
                          capacity_(capacity), map_(capacity) {}

  void add(KeyT key, T data) {
    if (map_.find(key) != map_.end()) {
      std::cerr << "Value already in cache\n";
      return;
    }

    if (size_ == capacity_) this->displace();

    FrqNodeIt first_frq = frq_nodes_.begin();

    if (first_frq == frq_nodes_.end() || first_frq->freq() != 0) {
      frq_nodes_.emplace(first_frq, FrqNode{});
      first_frq = frq_nodes_.begin();
    }

    ValNodeIt val = first_frq->add_val(ValNode{data, key, first_frq});

    map_.emplace(key, val);
    size_++;
  }

  template <DataAccessFunc AccessData>
  const T &get(KeyT key) {
    auto map_it = map_.find(key);

    if (map_it == map_.end()) {
      std::cerr << "key = " << key << " miss\n";
      this->add(key, AccessData(key));
      map_it = map_.find(key);
    } else {
      hits_++;
    }

    ValNodeIt val = inc_freq(map_it->second);
    // inc_freq makes map_it invalid
    map_it = map_.find(val->key());

    return map_it->second->data();
  }

  size_t freq(KeyT key) const {
    auto map_it = map_.find(key);
    if (map_it == map_.end()) return 0;
    return map_it->second->freq();
  }

  size_t hits() const { return hits_; }
  size_t size() const { return size_; }
  size_t capacity() const { return capacity_; }

  void dump(std::ostream &out) {
    out << "===================================================\n";
    out << "hits_     = " << hits_ << '\n';
    out << "size_     = " << size_ << '\n';
    out << "capacity_ = " << capacity_ << '\n';

    out << "MAP:\n";
    for (auto el : map_)
      out << '\t' << el.first << " -> " << el.second->data() << '\n';

    out << "FREQ NODES:\n";
    for (auto frq : frq_nodes_) {
      out << "\tfreq = " << frq.freq() << ": ";
      for (auto val : frq.vals()) out << val.key() << ' ';

      out << '\n';
    }
    out << "===================================================\n";
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
    if (frq->vals().size() == 0) frq_nodes_.erase(frq);

    // upd map_
    map_.erase(map_.find(new_val->key()));
    map_.emplace(new_val->key(), new_val);
    return new_val;
  }

  void displace() {
    KeyT displaced_key = frq_nodes_.front().displace();
    map_.erase(map_.find(displaced_key));
    size_--;

    std::cerr << "displace key = " << displaced_key << "\n";
  }

  class FrqNode {
    size_t freq_ = 0;
    std::list<ValNode> val_nodes_;

  public:
    FrqNode(size_t freq = 0) : freq_(freq) {}

    ValNodeIt add_val(ValNode &val, FrqNodeIt frq) {
      val.frq = frq;
      return val_nodes_.insert(val_nodes_.begin(), val);
    }

    //? i dont understand this rvalue ref stuff
    ValNodeIt add_val(ValNode &&val) {
      return val_nodes_.insert(val_nodes_.begin(), val);
    }

    void rm_val(ValNodeIt val) {
      val_nodes_.erase(val);
    }

    KeyT displace() {
      KeyT displaced_key = val_nodes_.back().key();
      std::cerr << "displacing " << displaced_key << '\n';
      val_nodes_.pop_back();
      return displaced_key;
    }

    const std::list<ValNode> &vals() const { return val_nodes_; }
    size_t freq() const { return freq_; }
  };

  class ValNode {
    T data_;
    KeyT key_;

  public:
    FrqNodeIt frq;

    ValNode(T data, KeyT key, FrqNodeIt frq) :
                       data_(data), key_(key), frq(frq) {}

    const T &data()   const { return data_; }
    const KeyT &key() const { return key_; }
    size_t freq()     const { return frq->freq(); }
  };

};

} // namespace caches
