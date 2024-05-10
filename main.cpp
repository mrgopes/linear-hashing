#include <iostream>
#include "ADS_set.h"

int main() {
  std::cout << "Hello, World!" << std::endl;

  std::vector<int> v1 {51, 47, 27, 7, 15, 31, 55};

  ADS_set<int> ht{};

  std::cout << "Vector size: " << v1.size() << std::endl;

  ht.insert(v1.begin(), v1.end());

  ht.dump(std::cout);

  return 0;
}
