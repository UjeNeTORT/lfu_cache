#include <iostream>
#include <list>
#include <unordered_map>

#include "cache.hpp"

int SlowGetPage(int key) {
    return key;
}

int main() {
    auto SlowGetPageLambda = [](int key){ return key; };
    caches::LFU_cache<int, int> cache {3};

    std::cout << "hits is     " << cache.hits()     << "\n";
    std::cout << "size is     " << cache.size()     << "\n";
    std::cout << "capacity is " << cache.capacity() << "\n";

    //cache.add<SlowGetPage>(1, 1);
    //cache.add(2, 2);
    //cache.add(3, 3);
    //cache.add(4, 4);

    std::cout << '\n';
    std::cout << "cache.get(1) = " << cache.get<SlowGetPageLambda>(1) << '\n';
    cache.dump(std::cout);
    std::cout << "cache.get(2) = " << cache.get<SlowGetPageLambda>(2) << '\n';
    cache.dump(std::cout);
    std::cout << "cache.get(3) = " << cache.get<SlowGetPageLambda>(3) << '\n';
    cache.dump(std::cout);
    std::cout << "cache.get(4) = " << cache.get<SlowGetPageLambda>(4) << '\n';
    cache.dump(std::cout);
    std::cout << "cache.get(4) = " << cache.get<SlowGetPageLambda>(4) << '\n';
    cache.dump(std::cout);
    std::cout << "cache.get(4) = " << cache.get<SlowGetPageLambda>(4) << '\n';
    cache.dump(std::cout);
    std::cout << '\n';
    std::cout << "cache.freq(1) = " << cache.freq(1) << '\n';
    std::cout << "cache.freq(2) = " << cache.freq(2) << '\n';
    std::cout << "cache.freq(3) = " << cache.freq(3) << '\n';
    std::cout << "cache.freq(4) = " << cache.freq(4) << '\n';
    std::cout << '\n';
    std::cout << "hits is     " << cache.hits()     << "\n";
    std::cout << "size is     " << cache.size()     << "\n";
    std::cout << "capacity is " << cache.capacity() << "\n";

    // TODO make val node and freq node private

    return 0;
}
