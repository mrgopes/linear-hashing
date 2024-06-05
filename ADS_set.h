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
      insert(ilist.begin(), ilist.end());
    }

    class Bucket {
    private:
        void erase(size_t pos) {
          if (pos >= sz || sz >= N) return;
          for (size_t i {pos}; i < sz - 1; ++i) {
            inhalt[i] = inhalt[i + 1];
          }
          --sz;
        }
    public:
        Key inhalt[N];
        Bucket* ueberlauf;
        ADS_set* parent;

        size_t sz;

        explicit Bucket(ADS_set* parent = nullptr): inhalt{}, ueberlauf{nullptr}, parent{parent}, sz{0} {}

        // todo: sehr sus, please fix
        Bucket& operator=(const Bucket& b) {
          if (this == &b) return *this;
          for (size_t i {0}; i < b.sz; ++i) {
            inhalt[i] = b.inhalt[i];
          }
          if (b.ueberlauf && !ueberlauf) ueberlauf = new Bucket{};
          *ueberlauf = *b.ueberlauf;
          sz = b.sz;
          parent = b.parent;
          return *this;
        }

        bool operator==(const Bucket& b) {
          return ueberlauf == b.ueberlauf && parent == b.parent && inhalt == b.inhalt && sz == b.sz;
        }

        ~Bucket() {
          delete ueberlauf;
        }

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

        key_type* find(key_type key) {
          for (size_t i {0}; i < sz; i++) {
            if (key_equal{}(inhalt[i], key)) return &(inhalt[i]);
          }
          Bucket* ueb {ueberlauf};
          while (ueb != nullptr) {
            for (size_t j {0}; j < ueb->sz; j++) {
              if (key_equal{}(ueb->inhalt[j], key)) return &ueb->inhalt[j];
            }
            ueb = ueb->ueberlauf;
          }
          return nullptr;
        }

        void set_parent(ADS_set* prnt) {
          parent = prnt;
        }

        Bucket(std::initializer_list<key_type> ilist): Bucket() {
          insert(ilist);
        }

        key_type* insert(Key item, bool allow_split = true) {
          for (size_t i {0}; i < sz; i++) {
            if (key_equal{}(inhalt[i], item)) return nullptr;
          }
          if (!full()) {
            inhalt[sz++] = item;
            return &(inhalt[sz]);
          } else {
            if (!ueberlauf) ueberlauf = new Bucket(nullptr);
            key_type* result = ueberlauf->insert(item, false);
            if (result != nullptr && allow_split) parent->global_split();
            return result;
          }
        }

        bool full() {
          return sz == N;
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
          Bucket* first_ueb{ueberlauf};
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
          delete first_ueb;
          parent->nextToSplit++;
          if (parent->nextToSplit == binpow(parent->d)) {
            parent->split_weiter();
          }
        }

        // todo ueberlauf
        size_t find_element(key_type b) {
          for (size_t i{0}; i < sz; ++i) {
            if (inhalt[i] == b) return i;
          }
          return sz;
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
      delete[] backup;
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

    ADS_set(const ADS_set &other): ADS_set() {
      // .. fuegt jeden Element von other ins this hinzu
    }

    ~ADS_set() {
      delete[] inhalt;
    }

    //ADS_set &operator=(const ADS_set &other);
    //ADS_set &operator=(std::initializer_list<key_type> ilist);

    size_type size() const {
      return sz;
    }
    bool empty() const {
      return sz == 0;
    }

    std::pair<iterator,bool> insert(const key_type &key) {
      size_t wert {get_hash_wert(key, d)};
      if (wert < nextToSplit) sz += (inhalt[get_hash_wert(key, d + 1)].insert(key) != nullptr);
      else sz += (inhalt[wert].insert(key) != nullptr);

      return {find(key), true};
    }

    template<typename InputIt> void insert(InputIt first, InputIt last) {
      for (auto it {first}; it != last; ++it) {
//        dump();
        size_t wert {get_hash_wert(*it, d)};
        if (wert < nextToSplit) sz += inhalt[get_hash_wert(*it, d + 1)].insert(*it) != nullptr;
        else sz += inhalt[wert].insert(*it) != nullptr;
      }
    }

    //void clear();
    //size_type erase(const key_type &key);

    size_type count(const key_type &key) const {
      size_t wert {get_hash_wert(key, d)};
      if (wert < nextToSplit) return inhalt[get_hash_wert(key, d + 1)].find((key)) != nullptr;
      else return inhalt[wert].find((key)) != nullptr;
    }

    iterator find(const key_type &key) const {
      size_t wert {get_hash_wert(key, d)};
      if (wert < nextToSplit)
        return {
        inhalt[get_hash_wert(key, d + 1)].find(key),
        inhalt + get_hash_wert(key, d + 1),
        inhalt[get_hash_wert(key, d + 1)].find_element(key), get_hash_wert(key, d + 1), max_sz, sz};
      else return {inhalt[wert].find(key), inhalt + wert, inhalt[wert].find_element(key), wert, max_sz, sz};
    }

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

    /*
     * EXTRA
     */

    static size_t binpow(size_t power) {
      return 1 << power;
    }

    size_t find_bucket(Bucket* b) {
      for (size_t i{0}; i < max_sz; ++i) {
        if (inhalt[i] == b) return i;
      }
      return max_sz;
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
private:
  pointer ptr;
  Bucket* bucket;
  size_t counter;
  size_t bucket_counter;
  size_t bucket_count;
  size_t el_count;

public:

  explicit ForwardIterator():
    ptr{nullptr}, bucket{nullptr}, counter{0}, bucket_counter{0}, bucket_count{0}, el_count{0}  {}

  ForwardIterator(pointer val, Bucket* bkt, size_t counter, size_t bucket_counter, size_t bucket_count, size_t el_count):
      ptr{val}, bucket{bkt}, counter{counter}, bucket_counter{bucket_counter}, bucket_count{bucket_count}, el_count{el_count}  {}
/*  reference operator*() const;
  pointer operator->() const;
  ForwardIterator &operator++();
  ForwardIterator operator++(int);
  friend bool operator==(const ForwardIterator &lhs, const ForwardIterator &rhs);
  friend bool operator!=(const ForwardIterator &lhs, const ForwardIterator &rhs);*/
};

template <typename Key, size_t N>
void swap(ADS_set<Key,N> &lhs, ADS_set<Key,N> &rhs) { lhs.swap(rhs); }

//template <typename Key, size_t N>
//size_t ADS_set<Key, N>::d = 1;

#endif // ADS_SET_H