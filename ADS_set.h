#ifndef ADS_SET_H
#define ADS_SET_H

#include <functional>
#include <algorithm>
#include <iostream>
#include <stdexcept>

template <typename Key, size_t N = 3/* default N (implementation-defined) */>
class ADS_set {
public:
  //class ForwardIterator /* iterator type (implementation-defined) */;
  using value_type = Key;
  using key_type = Key;
  using reference = value_type &;
  using const_reference = const value_type &;
  using size_type = size_t;
  using difference_type = std::ptrdiff_t;
  //using const_iterator = const ForwardIterator/* iterator type */;
  //using iterator = const_iterator;
  using key_equal = std::equal_to<key_type>;                       // Hashing
  using hasher = std::hash<key_type>;                              // Hashing

  void insert(std::initializer_list<key_type> ilist) {
    insert(ilist.begin(), ilist.end());
  }

  class Bucket {
  public:
    Key inhalt[N];
    Bucket* ueberlauf;
    ADS_set* parent;

    size_t sz;

    explicit Bucket(ADS_set* parent = nullptr): inhalt{}, ueberlauf{nullptr}, parent{parent}, sz{0} {}

    friend std::ostream& operator<<(std::ostream& os, const Bucket &rop) {
      os << "{ Bucket: ";
      for (size_t i {0}; i < rop.sz; ++i) {
        os << "[" << i << "] " << rop.inhalt[i] << ", ";
      }
      if (rop.ueberlauf != nullptr) {
        os << "Ueberlauf: " << *(rop.ueberlauf);
      }
      return os << "}";
    }

    bool find(key_type key) const {
      for (size_t i {0}; i < sz; i++) {
        if (key_equal{}(inhalt[i], key)) return true;
      }
      Bucket* ueb {ueberlauf};
      while (ueb != nullptr) {
        for (size_t j {0}; j < ueb->sz; j++) {
          if (key_equal{}(ueb->inhalt[j], key)) return true;
        }
        ueb = ueb->ueberlauf;
      }
      return false;
    }

    void set_parent(ADS_set* prnt) {
      parent = prnt;
    }

    Bucket(std::initializer_list<key_type> ilist): Bucket() {
      insert(ilist);
    }

    bool insert(Key item, bool allow_split = true) {
      for (size_t i {0}; i < sz; i++) {
        if (key_equal{}(inhalt[i], item)) return false;
      }
      if (!full()) {
        inhalt[sz++] = item;
        return true;
      } else {
        if (!ueberlauf) ueberlauf = new Bucket(nullptr);
        bool result = ueberlauf->insert(item, false);
        if (result && allow_split) parent->global_split();
        return result;
      }
    }

    bool full() {
      return sz == N;
    }

    void erase(size_t pos) {
      for (size_t i {pos}; i < sz - 1; ++i) {
        inhalt[i] = inhalt[i + 1];
      }
      --sz;
    }

    void split() {
      if (!parent) return;
      for (size_t i {sz}; i > 0; --i) {
        if (get_hash_wert(inhalt[i - 1], parent->d + 1) != get_hash_wert(inhalt[i - 1], parent->d)) {
          parent->inhalt[get_hash_wert(inhalt[i - 1], parent->d+1)].insert(inhalt[i - 1], false);
          erase(i - 1);
        }
      }
      Bucket* ueb {ueberlauf};
      ueberlauf = nullptr;
      while (ueb != nullptr) {
        for (size_t i {ueb->sz}; i > 0; --i) {
          if (get_hash_wert(ueb->inhalt[i - 1], parent->d + 1) != get_hash_wert(ueb->inhalt[i - 1], parent->d)) {
            parent->inhalt[get_hash_wert(ueb->inhalt[i - 1], parent->d+1)].insert(ueb->inhalt[i - 1], false);
            ueb->erase(i - 1);
          } else {
            this->insert(ueb->inhalt[i - 1], false);
          }
        }
        // ... vllt irgendwann mal den alten ueb deleten?
        ueb = ueb->ueberlauf;
      }
      parent->nextToSplit++;
      if (parent->nextToSplit == binpow(parent->d)) {
        parent->split_weiter();
      }
    }
  };

  size_t d;
  size_t max_sz;
  Bucket* inhalt;
  size_t nextToSplit;
  size_t sz;

  ADS_set(): d{0}, max_sz{2}, inhalt{new Bucket[2]}, nextToSplit{0}, sz{0} {
    inhalt->set_parent(this);
    inhalt[1].set_parent(this);
  }

  void global_split() {
    inhalt[nextToSplit].split();
  }

  void split_weiter() {
    nextToSplit = 0;
    d++;
    max_sz = binpow(d + 1);

    Bucket* backup = inhalt;
    inhalt = new Bucket[binpow(d + 1)];
    for (size_t i {0}; i < binpow(d); ++i) {
      inhalt[i] = backup[i];
    }
    for (size_t i {binpow(d)}; i < binpow(d + 1); ++i) {
      inhalt[i].set_parent(this);
    }
  }

  ADS_set(std::initializer_list<key_type> ilist): ADS_set() {
    insert(ilist.begin(), ilist.end());
  }

  static size_t get_hash_wert(const key_type& item, size_t d) {
    return hasher{}(item) & ((1 << d) - 1);
  }

  template<typename InputIt> ADS_set(InputIt first, InputIt last) : ADS_set() {
    insert(first, last);
  }

  //ADS_set(const ADS_set &other);

  //~ADS_set() {}

  //ADS_set &operator=(const ADS_set &other);
  //ADS_set &operator=(std::initializer_list<key_type> ilist);

  size_type size() const {
    return sz;
  }
  bool empty() const {
    return sz == 0;
  }

  /*
  std::pair<iterator,bool> insert(const key_type &key) {
    size_t wert {get_hash_wert(key, d)};
    if (wert < nextToSplit) sz += inhalt[get_hash_wert(key, d + 1)].insert(key);
    else sz += inhalt[wert].insert(key);
  }*/

  template<typename InputIt> void insert(InputIt first, InputIt last) {
    for (auto it {first}; it != last; ++it) {
      size_t wert {get_hash_wert(*it, d)};
      if (wert < nextToSplit) sz += inhalt[get_hash_wert(*it, d + 1)].insert(*it);
      else sz += inhalt[wert].insert(*it);
    }
  }

  //void clear();
  //size_type erase(const key_type &key);

  size_type count(const key_type &key) const {
    size_t wert {get_hash_wert(key, d)};
    if (wert < nextToSplit) return inhalt[get_hash_wert(key, d + 1)].find((key));
    else return inhalt[wert].find((key));
  }

  //iterator find(const key_type &key) const;

  //void swap(ADS_set &other);

  //const_iterator begin() const;
  //const_iterator end() const;

  void dump(std::ostream &o = std::cerr) const {
    for (size_t i {0}; i < max_sz; ++i) {
      o << inhalt[i] << std::endl;
    }
    o << "Size: " << size() << std::endl;
  }

  /*
  friend bool operator==(const ADS_set &lhs, const ADS_set &rhs);
  friend bool operator!=(const ADS_set &lhs, const ADS_set &rhs);
   */

  static size_t binpow(size_t power) {
    return 1 << power;
  }
};

/*
template <typename Key, size_t N>
class ADS_set<Key,N>::ForwardIterator {
public:
  using value_type = Key;
  using difference_type = std::ptrdiff_t;
  using reference = const value_type &;
  using pointer = const value_type *;
  using iterator_category = std::forward_iterator_tag;

  explicit ForwardIterator(*//* implementation-dependent *//*);
  reference operator*() const;
  pointer operator->() const;
  ForwardIterator &operator++();
  ForwardIterator operator++(int);
  friend bool operator==(const ForwardIterator &lhs, const ForwardIterator &rhs);
  friend bool operator!=(const ForwardIterator &lhs, const ForwardIterator &rhs);
};*/

template <typename Key, size_t N>
void swap(ADS_set<Key,N> &lhs, ADS_set<Key,N> &rhs) { lhs.swap(rhs); }

//template <typename Key, size_t N>
//size_t ADS_set<Key, N>::d = 1;

#endif // ADS_SET_H