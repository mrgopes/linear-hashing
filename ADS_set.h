#ifndef ADS_SET_H
#define ADS_SET_H

#include <functional>
#include <algorithm>
#include <iostream>
#include <stdexcept>

template <typename Key, size_t N = 3/* default N (implementation-defined) */>
class ADS_set {
public:
  class ForwardIterator /* iterator type (implementation-defined) */;
  using value_type = Key;
  using key_type = Key;
  using reference = value_type &;
  using const_reference = const value_type &;
  using size_type = size_t;
  using difference_type = std::ptrdiff_t;
  using const_iterator = const ForwardIterator/* iterator type */;
  using iterator = const_iterator;
  using key_equal = std::equal_to<key_type>;                       // Hashing
  using hasher = std::hash<key_type>;                              // Hashing

  class Bucket {
  public:
    Key inhalt[N];
    static size_t id;

    Bucket(): inhalt{} {}

    Bucket(std::initializer_list<key_type> ilist): inhalt{} {
      for (size_t i {0}; i < N && i < sizeof(ilist); i++) {
        inhalt[i] = ilist[i];
      }
    }

    bool full() {
      return sizeof(inhalt) / sizeof(key_type) == N;
    }
  };

  class Knoten {
  public:
    Knoten* eltern;
    bool rechts;
    Knoten* left;
    Knoten* right;
    Bucket* bucket;

    explicit Knoten(bool init = false, Knoten* eltern = nullptr, bool rechts = false): left{nullptr}, right{nullptr}, bucket{nullptr}, eltern{eltern},
    rechts{rechts} {
      if (init) {
        bucket = new Bucket();
      }
    }

    Bucket* generiere_unterknoten(bool init = false) {
      Bucket* backup {bucket};
      bucket = nullptr;
      left = new Knoten(init, *this, false);
      right = new Knoten(init, *this, true);
      return backup;
    }

    void insert(value_type item) {
      if (bucket) {
        if (bucket->full()) {
          // .. split
        } else {
          // .. insert
        }
      } else {
        // ... logik ob das n-te bit 1 oder 0 ist
        if (true) {
          left->insert(item);
        }
      }

    }
  };

  Knoten wurzel;

  ADS_set(): wurzel{false} {
    wurzel.generiere_unterknoten(true);
  }

  ADS_set(std::initializer_list<key_type> ilist){
    for (const auto& i : ilist) {

    }
  }

  template<typename InputIt> ADS_set(InputIt first, InputIt last);     // PH1
  ADS_set(const ADS_set &other);

  ~ADS_set();

  ADS_set &operator=(const ADS_set &other);
  ADS_set &operator=(std::initializer_list<key_type> ilist);

  size_type size() const;                                              // PH1
  bool empty() const;                                                  // PH1

  void insert(std::initializer_list<key_type> ilist);                  // PH1
  std::pair<iterator,bool> insert(const key_type &key);
  template<typename InputIt> void insert(InputIt first, InputIt last); // PH1

  void clear();
  size_type erase(const key_type &key);

  size_type count(const key_type &key) const;                          // PH1
  iterator find(const key_type &key) const;

  void swap(ADS_set &other);

  const_iterator begin() const;
  const_iterator end() const;

  void dump(std::ostream &o = std::cerr) const;

  friend bool operator==(const ADS_set &lhs, const ADS_set &rhs);
  friend bool operator!=(const ADS_set &lhs, const ADS_set &rhs);
};

template <typename Key, size_t N>
class ADS_set<Key,N>::ForwardIterator {
public:
  using value_type = Key;
  using difference_type = std::ptrdiff_t;
  using reference = const value_type &;
  using pointer = const value_type *;
  using iterator_category = std::forward_iterator_tag;

  explicit ForwardIterator(/* implementation-dependent */);
  reference operator*() const;
  pointer operator->() const;
  ForwardIterator &operator++();
  ForwardIterator operator++(int);
  friend bool operator==(const ForwardIterator &lhs, const ForwardIterator &rhs);
  friend bool operator!=(const ForwardIterator &lhs, const ForwardIterator &rhs);
};

template <typename Key, size_t N>
void swap(ADS_set<Key,N> &lhs, ADS_set<Key,N> &rhs) { lhs.swap(rhs); }

template <typename Key, size_t N>
size_t ADS_set<Key, N>::Bucket::id = 0;

#endif // ADS_SET_H
