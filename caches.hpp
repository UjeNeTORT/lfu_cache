#include <list>
#include <unordered_map>
#include <iostream> // todo: delete when debugging is over

namespace caches {

// is it ok to have same default capacity for all caches?
const int DFLT_CACHE_CAPACITY = 1000;

template <typename T>
class LRU_cache {

    std::list<T> list;
    std::unordered_map<int, T> map;
    int capacity;

public:
    LRU_cache() {
        list = std::list<T>();
        capacity = DFLT_CACHE_CAPACITY;
        map = std::unordered_map<int, T>(DFLT_CACHE_CAPACITY);
    }

    LRU_cache(int capacity) {
        list = std::list<T>();
        this->capacity = capacity;
        map = std::unordered_map<int, T>(this->capacity);
    }

    ~LRU_cache() = default;

    int  get_capacity() { return capacity; }
    int  get_size()     { return list.size(); }
    bool is_empty()     { return list.empty(); }
};
}