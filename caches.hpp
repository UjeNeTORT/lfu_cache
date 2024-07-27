#include <list>
#include <unordered_map>
#include <iostream> // todo: delete when debugging is over

// is it ok to have same default capacity for all caches?
const int DFLT_CACHE_CAPACITY = 1000;

namespace caches {

template <typename T>
class LRU_cache {

    std::list<T> list;
    std::unordered_map<int, T> map;
    int capacity;

public:
    LRU_cache() {
        list = std::list();
        map = std::unordered_map(DFLT_CACHE_CAPACITY);
    }

    LRU_cache(int capacity) {
        list = std::list();
        map = std::unordered_map(capacity);
    }

    ~LRU_cache() = default;

    int get_capacity() {
        return capacity;
    }

    int get_size() {
        return list.size();
    }

    bool is_empty() {
        return list.empty();
    }
};
}