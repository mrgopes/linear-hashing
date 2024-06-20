#include <iostream>
#include "ADS_set_temp.h"
#include <random>

using u32    = uint_least32_t;
using engine = std::mt19937;

int main() {
  std::cout << "Hello, World!" << std::endl;

  std::vector<size_t> v1 {17179869183, 8589934591};

  ADS_set<size_t, 100> ht{};
  ADS_set<size_t, 100> ht2{};

//  ht.insert(v1.begin(), v1.end());

  std::random_device os_seed;
  const u32 seed = os_seed();

  engine generator( seed );
  std::uniform_int_distribution< u32 > distribute( 1, 1000000);

  int tracker = 0;
  for (int i {0}; i < 100000; ++i) {
    size_t item {distribute( generator )};
    ht.insert(item);
    ht2.insert(item);
    tracker++;
  }

  std::cout << tracker << std::endl;

  ADS_set<size_t, 1> ht3{};

  //ht.dump();

//  auto it = ht.begin();

//  for (; it != ht.end(); it++) {
//    std::cout << *it << std::endl;
//  }

  //std::cout << *it << std::endl;

  //it = ht.end();

//  std::cout << *(it++) << std::endl;
//

  ht.insert(500);

  tracker = 0;
  std::cout << "Iterating over:" << std::endl;
  for (const auto& i : ht) {
//    std::cout << i << std::endl;
    tracker++;
  }

  auto j {ht.begin()};
  for (auto i {ht.begin()}; i != ht.end() && j != ht.end(); i++) {
    if (i != j) std::cout << "OIDA" << std::endl;
    j++;
  }

  std::cout << tracker << std::endl;

  for (int i {0}; i < 1000000; ++i) {
    size_t item {distribute( generator )};
    ht.erase(item);
  }

//  for (int i {7}; i < 2000; ++i) {
//    ht.erase(i);
//  }

//  ht2.swap(ht);

  ht2.clear();

  std::cout << "Size:" << std::endl;
  std::cout << std::distance(ht.begin(), ht.end()) << std::endl;
  std::cout << ht.size() << std::endl;

  if (ht.begin() == ht.end()) std::cout << "end" << std::endl;

  //ht.clear();

  //ht.dump();

  //int **i {new int*[2147483648]};

  //std::cout << "Vector size: " << v1.size() << std::endl;

//  ht.insert(v1.begin(), v1.end());

//  ht.dump(std::cout);
//  ht2.dump();

  //std::cout << ht.inhalt[4].find_element(500) << std::endl;

  ADS_set<size_t, 100> big{1, 2, 3, 4, 5, 6, 7, 8, 9, 0};
  ADS_set<size_t, 1> small{10, 2, 1000, 50};

  std::cout << "Iterating over:" << std::endl;
  for (const auto& i : small) {
    std::cout << i << " ";
//    tracker++;
  }
  std::cout << std::endl;

  return 0;
}
