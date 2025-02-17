#include <iostream>
#include <list>
#include <vector>
#include <unordered_map>

#include "cache.hpp"

int main() {
  auto SlowGetPage = [](const int &key) -> int { return key; };

  size_t belady_capacity = 0;
  int n_queries = 0;
  std::cin >> belady_capacity;
  std::cin >> n_queries;

  std::list<int> queries;

  unsigned curr = 0;
  for (int i = 0; i < n_queries; i++) {
    std::cin >> curr;
    queries.push_back(curr);
  }

  caches::Belady_cache<int, int> belady {belady_capacity, queries, SlowGetPage};
  belady.run();
  std::cout << belady.hits() << '\n';

  // std::cout << belady << '\n';

  return 0;
}
