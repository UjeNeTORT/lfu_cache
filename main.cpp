#include <iostream>
#include "caches.hpp"

int main() {

    caches::LRU_cache<int> cache(10);

    std::cout << "Size is " << cache.get_capacity() << "\n";

    return 0;
}