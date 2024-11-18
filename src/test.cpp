#include <gtest/gtest.h>

#include <fstream>
#include <filesystem>
#include <iostream>
#include <list>

#include "cache.hpp"

TEST(LFU, hits) {
  std::filesystem::path testcases = "../test/";

  auto SlowGetPage = [](const int &key) -> int { return key; };

  for (auto const &dir_entry : std::filesystem::directory_iterator(testcases)) {
    if (!dir_entry.is_regular_file()) continue;
    auto fpath = dir_entry.path();
    std::ifstream testfile;
    testfile.open(fpath);

    size_t capacity, n_queries, answer;
    int key;

    if (testfile.is_open())
      testfile >> capacity >> n_queries >> answer;

    caches::LFU_cache<int, int> cache {capacity, SlowGetPage};
    for (size_t i = 0; i < n_queries; i++) {
      testfile >> key;
      cache.get(key);
    }

    testfile.close();
    ASSERT_EQ(cache.hits(), answer);
  }
}

TEST(LFU, stress) {
  std::filesystem::path testcases = "../test/";

  auto SlowGetPage = [](const int &key) -> int { return key; };

  for (auto const &dir_entry : std::filesystem::recursive_directory_iterator(testcases)) {
    if (!dir_entry.is_regular_file()) continue;
    auto fpath = dir_entry.path();
    std::ifstream testfile;
    testfile.open(fpath);

    size_t capacity, n_queries, answer_not_used;
    int key;

    if (testfile.is_open())
      testfile >> capacity >> n_queries >> answer_not_used;

    std::cerr << fpath << ' ';

    caches::LFU_cache<int, int> cache {capacity, SlowGetPage};
    for (size_t i = 0; i < n_queries; i++) {
      testfile >> key;
      cache.get(key);
    }
    testfile.close();
    std::cerr << " (hits = " << cache.hits() << ")\n";
  }
}

TEST(belady, hits) {
  std::filesystem::path testcases = "../test/belady/";

  auto SlowGetPage = [](const int &key) -> int { return key; };

  for (auto const &dir_entry : std::filesystem::directory_iterator(testcases)) {
    if (!dir_entry.is_regular_file()) continue;
    auto fpath = dir_entry.path();
    std::ifstream testfile {fpath};

    size_t capacity, answer;
    int n_queries;
    int key;

    if (testfile.is_open())
      testfile >> capacity >> n_queries >> answer;

    std::list<int> queries {};
    for (int i = 0; i < n_queries; i++) {
      testfile >> key;
      queries.push_back(key);
    }

    caches::Belady_cache<int, int> beladka {capacity, queries, SlowGetPage};
    beladka.run();

    ASSERT_EQ(beladka.hits(), answer);
  }
}

TEST(belady, stress) {
  std::filesystem::path testcases = "../test/";

  auto SlowGetPage = [](const int &key) -> int { return key; };

  for (auto const &dir_entry : std::filesystem::recursive_directory_iterator(testcases)) {
    if (!dir_entry.is_regular_file()) continue;
    auto fpath = dir_entry.path();
    std::ifstream testfile {fpath};
    std::cerr << fpath << ' ';

    size_t capacity, answer_not_used;
    int n_queries;
    int key;

    if (testfile.is_open())
      testfile >> capacity >> n_queries >> answer_not_used;

    std::list<int> queries {};
    for (int i = 0; i < n_queries; i++) {
      testfile >> key;
      queries.push_back(key);
    }

    caches::Belady_cache<int, int> beladka {capacity, queries, SlowGetPage};
    beladka.run();
    std::cerr << " (hits = " << beladka.hits() << ")\n";
  }
}

TEST(cache_compare, lfu_less_hits_than_ideal) {
  std::filesystem::path testcases = "../test/";

  auto SlowGetPage = [](const int &key) -> int { return key; };

  for (auto const &dir_entry : std::filesystem::recursive_directory_iterator(testcases)) {
    if (!dir_entry.is_regular_file()) continue;
    auto fpath = dir_entry.path();
    std::ifstream testfile {fpath};
    std::cerr << fpath << ' ';

    size_t capacity, answer_not_used;
    int n_queries;
    int key;

    if (testfile.is_open())
      testfile >> capacity >> n_queries >> answer_not_used;

    caches::LFU_cache<int, int> lfu {capacity, SlowGetPage};

    std::list<int> queries {};
    for (int i = 0; i < n_queries; i++) {
      testfile >> key;
      queries.push_back(key);
      lfu.get(key);
    }

    caches::Belady_cache<int, int> beladka {capacity, queries, SlowGetPage};
    beladka.run();

    ASSERT_LE(lfu.hits(), beladka.hits());
  }

}

int main(int argc, char *argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
