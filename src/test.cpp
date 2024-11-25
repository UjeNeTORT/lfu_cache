#include <gtest/gtest.h>

#include <fstream>
#include <filesystem>
#include <iostream>
#include <list>

#include "cache.hpp"

class TestLFU : public ::testing::Test {

protected:
  using DataAccessFunc = int (*)(const int &);
  DataAccessFunc SlowGetPage = [](const int &key) -> int { return key; };

  std::filesystem::path testf_path_;
  size_t capacity_ = 0;
  size_t n_queries_ = 0;
  size_t answer_ = 0;
  caches::LFU_cache<int, int> *lfu_;

  virtual void SetUp() {
    lfu_ = new caches::LFU_cache<int, int>;
  }

  virtual void TearDown() {
    delete lfu_;
  }

  void RunTest(std::filesystem::path testf_path) {
    testf_path_ = testf_path;
    std::ifstream testfile{testf_path_};
    if (!testfile.is_open()) {
      std::cerr << "ERROR: could not open " << testf_path_ << '\n';
      return;
    }

    testfile >> capacity_ >> n_queries_ >> answer_;

    int curr_key = 0;
    *lfu_ = caches::LFU_cache<int, int> {capacity_, SlowGetPage};
    for (size_t i = 0; i < n_queries_; i++) {
      testfile >> curr_key;
      lfu_->get(curr_key);
    }
  }

  void TestHits(std::filesystem::path testf_path) {
    RunTest(testf_path);
    ASSERT_EQ(lfu_->hits(), answer_);
  }
};

class TestBelady : public ::testing::Test {

protected:
  using DataAccessFunc = int (*)(const int &);
  DataAccessFunc SlowGetPage = [](const int &key) -> int { return key; };

  std::filesystem::path testf_path_;
  size_t capacity_ = 0;
  size_t n_queries_ = 0;
  size_t answer_ = 0;
  caches::Belady_cache<int, int> *beladka_;

  virtual void SetUp() {
    beladka_ = new caches::Belady_cache<int, int>;
  }

  virtual void TearDown() {
    delete beladka_;
  }

  void RunTest(std::filesystem::path testf_path) {
    testf_path_ = testf_path;
    std::ifstream testfile{testf_path_};
    if (!testfile.is_open()) {
      std::cerr << "ERROR: could not open " << testf_path_ << '\n';
      return;
    }

    testfile >> capacity_ >> n_queries_ >> answer_;

    int curr_key = 0;
    std::list<int> queries{};
    for (size_t i = 0; i < n_queries_; i++) {
      testfile >> curr_key;
      queries.push_back(curr_key);
    }

    *beladka_ = caches::Belady_cache<int, int> {capacity_, queries, SlowGetPage};
    beladka_->run();
  }

  void TestHits(std::filesystem::path testf_path) {
    RunTest(testf_path);
    ASSERT_EQ(beladka_->hits(), answer_);
  }
};

TEST_F(TestLFU, hits) {
  std::filesystem::path test_dir = "../test/lfu";

  for (auto const &dir_entry : std::filesystem::directory_iterator(test_dir)) {
    if (!dir_entry.is_regular_file()) continue;
    auto fpath = dir_entry.path();
    TestHits(fpath);
  }
}

TEST_F(TestLFU, stress) {
  std::filesystem::path test_dir = "../test/";

  for (auto const &dir_entry : std::filesystem::recursive_directory_iterator(test_dir)) {
    if (!dir_entry.is_regular_file()) continue;
    auto fpath = dir_entry.path();
    RunTest(fpath);
  }
}

TEST_F(TestBelady, hits) {
  std::filesystem::path testcases = "../test/belady/";

  for (auto const &dir_entry : std::filesystem::directory_iterator(testcases)) {
    if (!dir_entry.is_regular_file()) continue;
    auto fpath = dir_entry.path();
    TestHits(fpath);
  }
}

TEST_F(TestBelady, stress) {
  std::filesystem::path testcases = "../test/";

  for (auto const &dir_entry : std::filesystem::recursive_directory_iterator(testcases)) {
    if (!dir_entry.is_regular_file()) continue;
    auto fpath = dir_entry.path();
    RunTest(fpath);
  }
}

TEST(ComapeCaches, lfu_less_hits_than_ideal) {
  std::filesystem::path testcases = "../test/";

  auto SlowGetPage = [](const int &key) -> int { return key; };

  for (auto const &dir_entry : std::filesystem::recursive_directory_iterator(testcases)) {
    if (!dir_entry.is_regular_file()) continue;
    auto fpath = dir_entry.path();
    std::ifstream testfile {fpath};
    // std::cerr << fpath << ' ';

    size_t capacity = 0;
    size_t answer_not_used = 0;
    int n_queries = 0;
    int key = 0;

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
