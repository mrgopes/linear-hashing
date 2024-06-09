#include <iostream>
#include "ADS_set_temp.h"

int main() {
  std::cout << "Hello, World!" << std::endl;

  std::vector<size_t> v1 {17179869183, 8589934591};

  ADS_set<size_t, 1> ht{};

//  ht.insert(v1.begin(), v1.end());



  for (int i {0}; i < 100; ++i) {
    ht.insert(i);
  }

  //ht.dump();

//  auto it = ht.begin();

//  for (; it != ht.end(); it++) {
//    std::cout << *it << std::endl;
//  }

  //std::cout << *it << std::endl;

  //it = ht.end();

//  std::cout << *(it++) << std::endl;
//
  for (const auto& i : ht) {
    std::cout << i << std::endl;
  }

  //int **i {new int*[2147483648]};

  //std::cout << "Vector size: " << v1.size() << std::endl;

//  ht.insert(v1.begin(), v1.end());

//  ht.dump(std::cout);

  return 0;
}
