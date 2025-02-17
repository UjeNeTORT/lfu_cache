#include <iostream>
#include <list>
#include <vector>
#include <unordered_map>

#include "cache.hpp"

int main() {
  auto SlowGetPage = [](const int &key) -> int { return key; };

  size_t lfu_capacity = 0;
  int n_queries = 0;
  std::cin >> lfu_capacity;
  std::cin >> n_queries;

  int curr = 0;
  caches::LFU_cache<int, int> cache {lfu_capacity, SlowGetPage};
  for (size_t i = 0; i < n_queries; i++) {
    std::cin >> curr;
    cache.get(curr);
  }

  std::cout << cache.hits() << '\n';

  // std::cout << cache << '\n';

  return 0;
}
