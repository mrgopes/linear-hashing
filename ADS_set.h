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
    public:
        Key inhalt[N];
        Bucket* ueberlauf;
        ADS_set* parent;

        size_t sz;

        void erase(size_t pos) {
          Bucket* temp {ueberlauf};
          if (pos >= sz && pos < N) return;
          if (pos >= N) {
            for (size_t i {1}; i < pos / N; ++i) {
              temp = temp->ueberlauf;
            }
            pos = pos % N;
            for (size_t i {pos}; i < temp->sz - 1; ++i) {
              temp->inhalt[i] = temp->inhalt[i + 1];
            }
            --(temp->sz);
          } else {
            for (size_t i {pos}; i < sz - 1; ++i) {
              inhalt[i] = inhalt[i + 1];
            }
            --sz;
          }
        }

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

        key_type* first() {
          return &(inhalt[0]);
        }

        key_type* last() {
          if (ueberlauf) {
            Bucket* temp {ueberlauf};
            while (temp) {
              if (temp->ueberlauf) {
                temp = temp->ueberlauf;
              } else {
                return &(inhalt[temp->sz]);
              }
            }
            return &(inhalt[temp->sz]);
          } else {
            return &(inhalt[sz]);
          }
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

        size_t find_element(key_type b) {
          for (size_t i{0}; i < sz; ++i) {
            if (key_equal{}(inhalt[i], b)) return i;
          }
          if (ueberlauf) {
            size_t counter {N};
            Bucket* ueb {ueberlauf};
            while (ueb != nullptr) {
              for (size_t i{0}; i < sz; ++i) {
                if (key_equal{}(ueb->inhalt[i], b)) return counter + i;
              }
              ueb = ueb->ueberlauf;
              counter += N;
            }
          }
          return SIZE_MAX;
        }

        size_t get_sz() {
          size_t size{0};
          Bucket* current{this};
          while (current->ueberlauf) {
            size += N;
            current = current->ueberlauf;
          }
          size += current->sz;

          return size;
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
      for (const auto& i : other) {
        insert(i);
      }
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

    size_type erase(const key_type &key) {
      size_t wert {get_hash_wert(key, d)};
      if (wert < nextToSplit) {
        if (inhalt[get_hash_wert(key, d + 1)].find_element(key) != SIZE_MAX) {
          --sz;
          inhalt[get_hash_wert(key, d + 1)].erase(inhalt[get_hash_wert(key, d + 1)].find_element(key));
          return 1;
        } else return 0;
      } else {
        if (inhalt[wert].find_element(key) != SIZE_MAX) {
          --sz;
          inhalt[wert].erase(inhalt[wert].find_element(key));
          return 1;
        } else {
          return 0;
        }
      }
    }

    size_type count(const key_type &key) const {
      size_t wert {get_hash_wert(key, d)};
      if (wert < nextToSplit) return inhalt[get_hash_wert(key, d + 1)].find((key)) != nullptr;
      else return inhalt[wert].find((key)) != nullptr;
    }

    iterator find(const key_type &key) const {
      size_t wert{get_hash_wert(key, d)};
      if (wert < nextToSplit) {
        if (inhalt[get_hash_wert(key, d + 1)].find_element(key) == SIZE_MAX)
          return {};
        return {
          inhalt[get_hash_wert(key, d + 1)].find(key),
          inhalt[get_hash_wert(key, d + 1)].find_element(key), get_hash_wert(key, d + 1), this};
      } else {
        if (inhalt[wert].find_element(key) == SIZE_MAX)
          return {};
        return {inhalt[wert].find(key), inhalt[wert].find_element(key), wert, this
        };
      }
    }

    //void swap(ADS_set &other);

    const_iterator begin() const {
      if (inhalt[0].get_sz() == 0) {
        ADS_set<Key, N>::ForwardIterator it {inhalt[0].first(), 0, 0, this};
        it++;
        return it;
      }
      else return {inhalt[0].first(), 0, 0, this};
    }
    const_iterator end() const {
      return {inhalt[max_sz - 1].last(), inhalt[max_sz - 1].find_element(*(inhalt[max_sz - 1].last())), max_sz - 1, this};
    }

    void dump(std::ostream &o = std::cerr) const {
      for (size_t i {0}; i < max_sz; ++i) {
        o << inhalt[i] << std::endl;
      }
      o << "Size: " << size() << std::endl;
    }

    friend bool operator==(const ADS_set &lhs, const ADS_set &rhs) {
      for (const auto& i : lhs) {
        if (rhs.find(i).is_leer()) return false;
      }
      if (lhs.max_sz != rhs.max_sz && lhs.sz != rhs.sz) return false;
      return true;
    }
    friend bool operator!=(const ADS_set &lhs, const ADS_set &rhs) {
      return !(lhs == rhs);
    }

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
  const ADS_set* parent;
  size_t counter;
  size_t bucket_counter;
  //size_t count;

public:

  ForwardIterator():
    ptr{nullptr}, counter{0}, bucket_counter{0}, parent{nullptr}  {}

  ForwardIterator(pointer val, size_t counter, size_t bucket_counter, const ADS_set* parent):
      ptr{val}, counter{counter}, bucket_counter{bucket_counter}, parent{parent}  {}
  reference operator*() const { return *ptr; }
  pointer operator->() const { return *ptr; }
  ForwardIterator &operator++() {
    if (*this != parent->end() && counter + 1 < parent->inhalt[bucket_counter].get_sz()) {
      ++counter;
    } else if (*this != parent->end() && bucket_counter != parent->max_sz - 1) {
      do {
        counter = 0;
        ++bucket_counter;
      } while (bucket_counter + 1 < parent->max_sz && counter >= parent->inhalt[bucket_counter].get_sz());
    }
    if (bucket_counter == parent->max_sz - 1 && counter + 1 >= parent->inhalt[bucket_counter].get_sz()) {
      counter = SIZE_MAX;
    }
    size_t temp_counter{counter};
    Bucket* target_b {&(parent->inhalt[bucket_counter])};
    if (counter >= N && counter != SIZE_MAX) {
      do {
        temp_counter -= N;
        target_b = parent->inhalt[bucket_counter].ueberlauf;
      } while (temp_counter >= target_b->sz);
    }
    if (counter != SIZE_MAX) ptr = target_b->inhalt + temp_counter;
    return *this;
  }
  ForwardIterator operator++(int) {
    ForwardIterator old {ptr, counter, bucket_counter, parent};
    this->operator++();
    return old;
  }

  bool is_leer() const {
    return !(ptr);
  }

  friend bool operator==(const ForwardIterator &lhs, const ForwardIterator &rhs) {
    return lhs.bucket_counter == rhs.bucket_counter && lhs.counter == rhs.counter;
  }
  friend bool operator!=(const ForwardIterator &lhs, const ForwardIterator &rhs) {
    return lhs.bucket_counter != rhs.bucket_counter || lhs.counter != rhs.counter;
  }
};

template <typename Key, size_t N>
void swap(ADS_set<Key,N> &lhs, ADS_set<Key,N> &rhs) { lhs.swap(rhs); }

//template <typename Key, size_t N>
//size_t ADS_set<Key, N>::d = 1;

#endif // ADS_SET_H