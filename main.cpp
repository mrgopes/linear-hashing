#include <iostream>
#include "ADS_set.h"

int main() {
  std::cout << "Hello, World!" << std::endl;

  std::vector<int> v1 {1, 78, 390, 1020};

  ADS_set<int> ht{};

  std::cout << ht.empty() << std::endl;

  ht.insert(v1.begin(), v1.end());




  std::cout << ht.size() << " " << ht.empty() << std::endl;

  return 0;
}
