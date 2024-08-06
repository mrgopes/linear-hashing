#include <iostream>
#include "ADS_set_temp.h"
#include <random>

using u32    = uint_least32_t;
using engine = std::mt19937;

int main() {
  std::cout << "Hello, World!" << std::endl;

  std::vector<size_t> v1 {17179869183, 8589934591};

  ADS_set<int, 2> ht{};

  for (int i {0}; i < 10; ++i) {
    ht.insert(i);
    ht.dump(std::cout);
    std::cout << ht.count(i) << std::endl;
  }

  for (const auto& i : ht) {
    std::cout << i << std::endl;
  }


  return 0;
}
