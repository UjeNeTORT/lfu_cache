#include <gtest/gtest.h>
#include <iostream>
#include <fstream>
#include <filesystem>

#include "cache.hpp"

TEST(LFU_hits, test) {
  std::filesystem::path testcases = "../test/";

  auto SlowGetPage = [](const int &key) -> int { return key; };

  for (auto const& dir_entry : std::filesystem::directory_iterator(testcases)) {
    auto fpath = dir_entry.path();
    std::ifstream testfile;
    testfile.open(fpath);

    size_t capacity, n_queries, answer;
    int key;

    if (testfile.is_open())
      testfile >> capacity >> n_queries >> answer;

    caches::LFU_cache<int, int> cache {capacity, SlowGetPage};
    for (int i = 0; i < n_queries; i++) {
      testfile >> key;
      cache.get(key);
    }

    testfile.close();
    ASSERT_EQ(cache.hits(), answer);
  }
}

//TEST(LFU_not_fall, test) {
  //caches::LFU_cache<int, int> cache {capacity, SlowGetPage};
  //bool all_ok = true;
  //for (int i = 0; i < n; i++)
    //cache.get(key);

  //ASSERT_EQ(all_ok, true);
//}

int main(int argc, char *argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
