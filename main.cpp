#include <iostream>
#include "ADS_set_temp.h"

int main() {
  std::cout << "Hello, World!" << std::endl;

  //std::vector<size_t> v1 {17179869183, 8589934591};

  ADS_set<size_t, 1> ht{};

  ht.insert(1);
  ht.insert(2);
  ht.insert(3);
  auto it = ht.insert(5);

  //int **i {new int*[2147483648]};

  //std::cout << "Vector size: " << v1.size() << std::endl;

//  ht.insert(v1.begin(), v1.end());

  ht.dump(std::cout);

  return 0;
}
