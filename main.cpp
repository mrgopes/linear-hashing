#include <iostream>
#include "ADS_set.h"

int main() {
  std::cout << "Hello, World!" << std::endl;

  //std::vector<size_t> v1 {17179869183, 8589934591};

  ADS_set<size_t, 1> ht{};
  std::vector<size_t> v1 {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

  //int **i {new int*[2147483648]};

  std::cout << "Vector size: " << v1.size() << std::endl;

  ht.insert(v1.begin(), v1.end());

  ht.dump(std::cout);

  return 0;
}
