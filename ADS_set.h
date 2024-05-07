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

  void insert(std::initializer_list<key_type> ilist) {
    for (const auto& i : ilist) {
      insert(i);
    }
  }

  class Bucket {
  public:
    Key inhalt[N];
    Bucket* ueberlauf;
    ADS_set* parent;

    explicit Bucket(ADS_set* parent = nullptr): parent{parent}, inhalt{}, ueberlauf{nullptr} {}

    void set_parent(ADS_set* prnt) {
      parent = prnt;
    }

    Bucket(std::initializer_list<key_type> ilist): inhalt{} {
      for (const auto& i : ilist) {
        insert(i);
      }
    }

    void insert(Key item, bool allow_split = true) {
      if (item == 0) return;
      if (!full()) {
        for (size_t i {0}; i < N; i++) {
          if (!(inhalt[i])) {
            std::cout << "Das item " << item << " wird eingefuegt zum Platz " << i << std::endl;
            inhalt[i] = item;
            std::cout << "Tada: " << inhalt[i] << std::endl;
            break;
          }
        }
      } else {
        std::cout << "full" << "\n";
        if (!ueberlauf) ueberlauf = new Bucket(nullptr);
        ueberlauf->insert(item);
        if (allow_split) parent->global_split();
      }
    }

    bool full() {
      for (size_t i {0}; i < N; i++) {
        if (!(inhalt[i])) {
          return false;
        }
      }
      return true;
    }

    void split() {
      if (!parent) return;
      for (size_t i {0}; i < N; i++) {
        if (get_hash_wert(inhalt[i], parent->d + 1) != get_hash_wert(inhalt[i], parent->d)) {
          parent->inhalt[get_hash_wert(inhalt[i], parent->d+1)].insert(inhalt[i], false);
          inhalt[i] = {};
        }
      }
      Bucket* ueb {ueberlauf};
      ueberlauf = nullptr;
      while (ueb != nullptr) {
        for (size_t i {0}; i < N; i++) {
          if (ueb->inhalt[i] == 0) continue;
          if (get_hash_wert(ueb->inhalt[i], parent->d + 1) != get_hash_wert(ueb->inhalt[i], parent->d)) {
            parent->inhalt[get_hash_wert(ueb->inhalt[i], parent->d+1)].insert(ueb->inhalt[i], false);
            ueb->inhalt[i] = {};
          } else {
            this->insert(ueb->inhalt[i], false);
          }
        }
        ueb = ueb->ueberlauf;
      }
      parent->nextToSplit++;
      if (parent->nextToSplit == pow(2, parent->d)) {
        parent->split_weiter();
      }
    }
  };

  size_t d;
  size_t max_sz;
  Bucket* inhalt;
  size_t nextToSplit;

  ADS_set(): d{0}, max_sz{2}, inhalt{new Bucket[2]}, nextToSplit{0} {
    inhalt->set_parent(this);
    inhalt[1].set_parent(this);
  }

  void global_split() {
    inhalt[nextToSplit].split();
    std::cout << "Splitting " << nextToSplit << std::endl;
  }

  void split_weiter() {
    nextToSplit = 0;
    d++;
    max_sz = pow(2, d + 1);

    Bucket* backup = inhalt;
    inhalt = new Bucket[pow(2, d + 1)];
    for (int i {0}; i < pow(2, d); ++i) {
      inhalt[i] = backup[i];
    }
    for (int i {pow(2, d)}; i < pow(2, d+1); ++i) {
      inhalt[i].set_parent(this);
    }
  }

  ADS_set(std::initializer_list<key_type> ilist){
    insert(ilist);
  }

  static int get_hash_wert(const key_type& item, size_t d) {
    return hasher{}(item) & ((1 << d) - 1);
  }

  template<typename InputIt> ADS_set(InputIt first, InputIt last);     // PH1
  ADS_set(const ADS_set &other);

  ~ADS_set() {}

  ADS_set &operator=(const ADS_set &other);
  ADS_set &operator=(std::initializer_list<key_type> ilist);

  size_type size() const {
    return max_sz;
  }                                              // PH1
  bool empty() const;                                                  // PH1

  std::pair<iterator,bool> insert(const key_type &key) {
    size_t wert {get_hash_wert(key, d)};
    if (wert < nextToSplit) inhalt[get_hash_wert(key, d + 1)].insert(key);
    else inhalt[wert].insert(key);
  }

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

  static size_t pow(size_t lop, size_t rop) {
    if (rop == 0) return 1;
    for (int i {0}; i < rop - 1; ++i) {
      lop *= lop;
    }
    return lop;
  }
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

//template <typename Key, size_t N>
//size_t ADS_set<Key, N>::d = 1;

#endif // ADS_SET_H