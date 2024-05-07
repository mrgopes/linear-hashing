#include <iostream>
#include "ADS_set.h"

int main() {
  std::cout << "Hello, World!" << std::endl;

  ADS_set<int> ht{};

  ht.insert(5);
  ht.insert(2);
  ht.insert(4);
  ht.insert(8);
  ht.insert(11);
  ht.insert(12);
  ht.insert(15);
  ht.insert(20);

  std::cout << "Tschuess!" << std::endl;

  return 0;
}
