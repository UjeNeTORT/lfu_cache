#include <iostream>
#include <unordered_map>
#include <list>
#include "caches.hpp"

int main() {

    caches::LRU_cache<int> cache = 100;

    std::cout << "Size is " << cache.get_capacity() << "\n";

    return 0;
}