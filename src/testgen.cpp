#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>

int main(int argc, char *argv[]) {

  if (argc != 5) {
    std::cerr << "Bad parameters! Correct format:\n"
              << "<testgen> \"new_test_path\" <capacity> <n_queries> <elem_threshold>\n";

    return 1;
  }

  std::filesystem::path test_path {argv[1]};
  std::ofstream test_file {test_path};

  size_t capacity = atoi(argv[2]);
  size_t n_queries = atoi(argv[3]);
  size_t elem_threshold = atoi(argv[4]);
  size_t answer_placeholder = 0; // will not be used during testing

  test_file << capacity << ' ' << n_queries << ' ' << answer_placeholder << '\n';

  for (int i = 0; i < n_queries; i++)
    test_file << rand() % elem_threshold << ' ';

  test_file << '\n';

  return 0;
}
