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
    using const_iterator = ForwardIterator;
    using iterator = ForwardIterator;
    using key_equal = std::equal_to<key_type>;                       // Hashing
    using hasher = std::hash<key_type>;                              // Hashing

private:
    class Bucket;

    size_t d;
    size_t max_sz;
    Bucket* inhalt;
    Bucket* overflows;
    size_t nextToSplit;
    size_t sz;
public:
    void insert(std::initializer_list<key_type> ilist) {
      insert(ilist.begin(), ilist.end());
    }

    ADS_set(): d{1}, max_sz{2}, inhalt{new Bucket[2]}, overflows{nullptr}, nextToSplit{0}, sz{0} {
    }
private:
    void global_split() {
      overflows_erweitern();
      Bucket::split(inhalt[nextToSplit++], this);
      if (nextToSplit == binpow(d)) {
        nextToSplit = 0;
        d++;

        overflow_zusammenfuegen();
      }
    }

    void overflows_erweitern() {
      ++max_sz;

      Bucket* backup = overflows;
      overflows = new Bucket[max_sz - binpow(d)];
      for (size_t i {0}; i < max_sz - binpow(d) - 1; ++i) {
        overflows[i] = backup[i];
      }

      delete[] backup;
    }

    void overflow_zusammenfuegen() {
      max_sz = binpow(d);

      Bucket* backup = inhalt;
      inhalt = new Bucket[max_sz];
      for (size_t i {0}; i < binpow(d - 1); ++i) {
        inhalt[i] = backup[i];
      }
      for (size_t i {binpow(d - 1)}; i < max_sz; ++i) {
        inhalt[i] = overflows[i - binpow(d-1)];
      }
      delete[] overflows;
      delete[] backup;

      overflows = nullptr;
    }
public:
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
      insert(other.begin(), other.end());
    }

    ~ADS_set() {
      delete[] inhalt;
      delete[] overflow;
    }

    ADS_set &operator=(const ADS_set &other) {
      if (this == &other) return *this;
      clear();
      insert(other.begin(), other.end());
      return *this;
    }
    ADS_set &operator=(std::initializer_list<key_type> ilist) {
      clear();
      insert(ilist.begin(), ilist.end());
      return *this;
    }

    size_type size() const {
      return sz;
    }
    bool empty() const {
      return sz == 0;
    }

    std::pair<iterator,bool> insert(const key_type &key) {
      size_t wert {get_hash_wert(key, d)};
      bool result;
      if (wert < nextToSplit && wert != get_hash_wert(key, d + 1)) {
        result = Bucket::insert(key, overflows[wert], this);
      }
      else {
        result = Bucket::insert(key, inhalt[wert], this);
      }
      sz += result;

      return {find(key), result};
    }

    template<typename InputIt>
    void insert(InputIt first, InputIt last) {
      for (auto it {first}; it != last; ++it) {
        insert(*it);
      }
    }

    void clear() {
      sz = 0;
      nextToSplit = 0;
      d = 1;
      max_sz = binpow(d);

      Bucket* backup = inhalt;
      inhalt = new Bucket[binpow(d)];
      delete[] overflows;
      overflows = nullptr;
      delete[] backup;
    }

    size_type erase(const key_type &key) {
      size_t wert {get_hash_wert(key, d)};
      if (wert < nextToSplit && wert != get_hash_wert(key, d + 1)) {
        if (overflows[wert].find_original(key) == true) {
          --sz;
          overflows[wert].erase(overflows[wert].find_element(key));
          return 1;
        }
      } else {
        if (inhalt[wert].find_original(key) == true) {
          --sz;
          inhalt[wert].erase(inhalt[wert].find_element(key));
          return 1;
        }
      }
      return 0;
    }

    size_type count(const key_type &key) const {
      auto result {find(key)};
      return !result.is_leer();
    }

    iterator find(const key_type &key) const {
      size_t wert{get_hash_wert(key, d)};
      if (wert < nextToSplit && wert != get_hash_wert(key, d + 1)) {
        if (overflows[wert].find_original(key) == false)
          return end();
        return {
            overflows[wert].find(key),
            overflows[wert].find_element(key), get_hash_wert(key, d + 1), this};
      } else {
        if (inhalt[wert].find_original(key) == false)
          return end();
        return {inhalt[wert].find(key), inhalt[wert].find_element(key), wert, this
        };
      }
    }

    /*
     *     size_t d;
    size_t max_sz;
    Bucket* inhalt;
    size_t nextToSplit;
    size_t sz;
     */

    void swap(ADS_set &other) {
      std::swap(d, other.d);
      std::swap(max_sz, other.max_sz);
      std::swap(nextToSplit, other.nextToSplit);
      std::swap(inhalt, other.inhalt);
      std::swap(overflows, other.overflows);
      std::swap(sz, other.sz);
    }

    const_iterator begin() const {
      if (inhalt[0].get_sz() == 0) {
        const_iterator it {inhalt[0].first(), 0, 0, this};
        it++;
        return it;
      }
      else return const_iterator{inhalt[0].first(), 0, 0, this};
    }
    const_iterator end() const {
      return const_iterator{nullptr, SIZE_MAX, max_sz, this};
    }

    void dump(std::ostream &o = std::cerr) const {
      o << "d: " << d << ", allozierte Buckets insgesamt: " << max_sz << std::endl;
      for (size_t i {0}; i < binpow(d); ++i) {
        o << "Bucket Nr. " << i << " lokales d: " << ((i >= nextToSplit && i < binpow(d)) ? d : d+1) << " " << inhalt[i] << std::endl;
      }
      o << "Splitting buckets: " << std::endl;
      for (size_t i {binpow(d)}; i < max_sz; ++i) {
        o << "Bucket Nr. " << i << " " << overflows[i - binpow(d)] << std::endl;
      }
      o << "Size: " << size() << std::endl;
    }

    friend bool operator==(const ADS_set &lhs, const ADS_set &rhs) {
      for (const auto& i : lhs) {
        if (rhs.find(i).is_leer()) {
          return false;
        }
      }
      if (lhs.sz != rhs.sz) return false;
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

    size_t find_bucket(Bucket* b) const {
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

public:

    ForwardIterator():
        ptr{nullptr}, parent{nullptr}, counter{0}, bucket_counter{0}  {}

    ForwardIterator(pointer val, size_t counter, size_t bucket_counter, const ADS_set* parent):
        ptr{val}, parent{parent}, counter{counter}, bucket_counter{bucket_counter}  {}
    reference operator*() const { return *ptr; }
    pointer operator->() const { return ptr; }
    ForwardIterator &operator++() {
      Bucket* buckets {nullptr};
      if (bucket_counter >= parent->binpow(parent->d)) {
        buckets = parent->overflows;
      } else {
        buckets = parent->inhalt;
      }

//    std::cout << "i" << *ptr << std::endl;
      if (bucket_counter >= parent->max_sz) return *this;
      if (counter + 1 < buckets[bucket_counter % parent->binpow(parent->d)].get_sz()) {
        ++counter;
      } else if (counter + 1 >= buckets[bucket_counter % parent->binpow(parent->d)].get_sz()) {
        do {
          counter = 0;
          ++bucket_counter;
          if (bucket_counter >= parent->binpow(parent->d)) {
            buckets = parent->overflows;
          } else {
            buckets = parent->inhalt;
          }
        } while (bucket_counter < parent->max_sz && counter >= buckets[bucket_counter % parent->binpow(parent->d)].get_sz());
      }
      if (bucket_counter < parent->max_sz) {
        size_t temp_counter{counter};
        Bucket *target_b{&(buckets[bucket_counter % parent->binpow(parent->d)])};
        if (counter >= N) {
          do {
            temp_counter -= N;
            target_b = target_b->ueberlauf;
          } while (temp_counter >= target_b->sz);
        }
        ptr = target_b->inhalt + temp_counter;
      }
      if (bucket_counter == parent->max_sz) {
        ptr = nullptr;
      }
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
      return lhs.bucket_counter == rhs.bucket_counter && lhs.ptr == rhs.ptr;
    }
    friend bool operator!=(const ForwardIterator &lhs, const ForwardIterator &rhs) {
      return !(lhs == rhs);
    }
};

template <typename Key, size_t N>
void swap(ADS_set<Key,N> &lhs, ADS_set<Key,N> &rhs) { lhs.swap(rhs); }

//template <typename Key, size_t N>
//size_t ADS_set<Key, N>::d = 1;

template <typename Key, size_t N>
class ADS_set<Key, N>::Bucket  {
public:
    Key inhalt[N];
    Bucket* ueberlauf;
    ADS_set* parent;

    size_t sz;

    void erase(size_t pos) {
      if (pos >= sz && pos < N) return;
      if (pos < N && N > 1) {
        for (size_t i {pos}; i < sz - 1; ++i) {
          if (i + 1 < N) inhalt[i] = inhalt[i + 1];
        }

        if (ueberlauf) {
          if (sz - 1 < N) inhalt[sz - 1] = ueberlauf->inhalt[0];
          if (ueberlauf->sz == 1 && ueberlauf->ueberlauf == nullptr) {
            delete ueberlauf;
            ueberlauf = nullptr;
          } else {
            ueberlauf->erase(0);
          }
        } else {
          --sz;
        }
      }
      else if (pos < N && N == 1) {
        if (ueberlauf) {
          inhalt[0] = ueberlauf->inhalt[0];
          if (ueberlauf->ueberlauf) {
            Bucket* temp {ueberlauf};
            ueberlauf = temp->ueberlauf;
            temp->ueberlauf = nullptr;
            delete temp;
          } else {
            delete ueberlauf;
            ueberlauf = nullptr;
          }
        } else {
          --sz;
        }
      }
      else if (ueberlauf && pos == N && ueberlauf->sz == 1 && ueberlauf->ueberlauf == nullptr) {
        delete ueberlauf;
        ueberlauf = nullptr;
      }
      else if (pos >= N && ueberlauf) {
        ueberlauf->erase(pos - N);
      }
    }

    explicit Bucket(ADS_set* parent = nullptr): inhalt{}, ueberlauf{nullptr}, parent{parent}, sz{0} {}

    Bucket& operator=(const Bucket& b) {
      if (this == &b) return *this;
      for (size_t i {0}; i < b.sz; ++i) {
        inhalt[i] = b.inhalt[i];
      }
      if (b.ueberlauf && !ueberlauf)
      {
        ueberlauf = new Bucket{};
        *ueberlauf = *b.ueberlauf;
      }
      sz = b.sz;
      return *this;
    }

    bool operator==(const Bucket& b) {
      return ueberlauf == b.ueberlauf && inhalt == b.inhalt && sz == b.sz;
    }

    ~Bucket() {
      delete ueberlauf;
    }

    friend std::ostream& operator<<(std::ostream& os, const Bucket &rop) {
      os << "{ Sz: " << rop.sz << " Bucket: ";
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

    bool find_original(key_type key) const {
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

    Bucket(std::initializer_list<key_type> ilist): Bucket() {
      insert(ilist);
    }

    // auslagern
    static bool insert(Key item, Bucket& b, ADS_set* s, bool allow_split = true) {
      for (size_t i {0}; i < b.sz; i++) {
        if (key_equal{}(b.inhalt[i], item)) return false;
      }
      if (!b.full()) {
        b.inhalt[(b.sz)++] = item;
        return &(b.inhalt[b.sz]);
      } else {
        if (!b.ueberlauf) b.ueberlauf = new Bucket(nullptr);
        bool result = b.ueberlauf->insert(item, *(b.ueberlauf), s, false);
        if (result && allow_split) s->global_split();
        return result;
      }
    }

    bool full() {
      return sz == N;
    }

    // auslagern
    static void split(Bucket& b, ADS_set* s) {
      Bucket* ueb {b.ueberlauf};
      Bucket* first_ueb{b.ueberlauf};
      b.ueberlauf = nullptr;
      for (size_t i {0}; i < b.sz;) {
        if (get_hash_wert(b.inhalt[i], s->d + 1) != get_hash_wert(b.inhalt[i], s->d)) {
          s->overflows[get_hash_wert(b.inhalt[i], s->d+1) - binpow(s->d)].insert(b.inhalt[i], s->overflows[get_hash_wert(b.inhalt[i], s->d+1) - binpow(s->d)], s, false);
          b.erase(i);
        } else {
          ++i;
        }
      }
      while (ueb != nullptr) {
        for (size_t i {0}; i < ueb->sz; i++) {
          if (get_hash_wert(ueb->inhalt[i], s->d + 1) != get_hash_wert(ueb->inhalt[i], s->d)) {
            s->overflows[get_hash_wert(ueb->inhalt[i], s->d+1) - binpow(s->d)].insert(ueb->inhalt[i], s->overflows[get_hash_wert(ueb->inhalt[i], s->d+1) - binpow(s->d)], s, false);
          } else {
            b.insert(ueb->inhalt[i], b, s, false);
          }
        }
        // ... vllt irgendwann mal den alten ueb deleten?
        ueb = ueb->ueberlauf;
      }
      delete first_ueb;
    }

    size_t find_element(key_type b) {
      for (size_t i{0}; i < sz; ++i) {
        //if (key_equal{}(inhalt[i], b)) return i;
        if (key_equal{}(inhalt[i], b)) return i;
      }
      if (ueberlauf) {
        size_t counter {N};
        Bucket* ueb {ueberlauf};
        while (ueb != nullptr) {
          for (size_t i{0}; i < ueb->sz; ++i) {
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

#endif // ADS_SET_H