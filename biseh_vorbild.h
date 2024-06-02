#ifndef ADS_SET_H
#define ADS_SET_H

// ADS_set.h BISEH
//
// VU Algorithmen und Datenstrukturen 1 - SS 2024 Universitaet Wien
// https://cewebs.cs.univie.ac.at/algodat/ss24/
//
// Beispielimplementierung mit "BISEH - Bounded Index Size Extendible Hashing"
// Erstellt in den Lehrveranstaltungen im SS 2021
// 04.05.2021 (1. Abgabe)
// 11.05.2021 (voller Funktionsumfang)
//
// Nicht optimiert und eventuell fehlerhaft (Fehler bitte melden)
//
// Die Zurverfügungstellung des Programmes oder Teilen davon auf anderen Plattformen,
// Repositories, etc. ist nicht zulässig.
//
// Dringende Empfehlung: Verwenden Sie diese Implementierung NICHT als
// Kopiervorlage für Ihre eigene Implementierung! Entwickeln Sie Ihre eigene
// Implementierung von Grund auf neu.

#include <functional>
#include <algorithm>
#include <iostream>
#include <stdexcept>

template <typename Key, size_t N = 13>
class ADS_set {
public:
    class Iterator;
    using value_type = Key;
    using key_type = Key;
    using reference = key_type &;
    using const_reference = const key_type &;
    using size_type = size_t;
    using difference_type = std::ptrdiff_t;
    using iterator = Iterator;
    using const_iterator = Iterator;
    using key_compare = std::less<key_type>;    // B+-Tree
    using key_equal = std::equal_to<key_type>;  // Hashing
    using hasher = std::hash<key_type>;         // Hashing
private:
    static constexpr size_t XBITS {2};
    static constexpr size_t YBITS {2};
    static constexpr size_t NX {1<<XBITS};
    static constexpr size_t NY {1<<YBITS};
    struct Hash;
    struct Address;
    struct Block;
    struct Entry;
    Entry **entries;
    size_t curr_size;
public:
    ADS_set(): entries{new Entry*[NX]}, curr_size{0} {
      for (size_t x{0}; x < NX; ++x) entries[x] = new Entry[NY];
    }
    ADS_set(std::initializer_list<key_type> ilist): ADS_set{std::begin(ilist),std::end(ilist)} {}
    template<typename InputIt> ADS_set(InputIt first, InputIt last): ADS_set{} {
      insert(first, last);
    }
    ADS_set(const ADS_set &other): ADS_set{other.begin(), other.end()} { // optimierbar
    }

    ~ADS_set() {
      for (size_t x{0}; x < NX; ++x) delete[] entries[x];
      delete[] entries;
    }

    ADS_set &operator=(const ADS_set &other) {
      if (this == &other) return *this;
      ADS_set tmp{other};
      swap(tmp);
      return *this;
    }
    ADS_set &operator=(std::initializer_list<key_type> ilist) {
      ADS_set tmp{ilist};
      swap(tmp);
      return *this;
    }

    size_type size() const {
      return curr_size;
    }
    bool empty() const {
      return !curr_size;
    }

    void insert(std::initializer_list<key_type> ilist) {
      insert(std::begin(ilist),std::end(ilist));
    }
    std::pair<iterator,bool> insert(const key_type &k) {
      if (Address a{k,this}) {
        return {{entries, a}, false};
      } else {
        ++curr_size;
        return {{entries, {a.h, entries[a.h.x][a.h.y].append(k,a.h.r)}}, true};
      }
    }
    template<typename InputIt> void insert(InputIt first, InputIt last) {
      for (auto it{first}; it != last; ++it) {
        if (Address a{*it, this}) {
        } else {
          ++curr_size;
          entries[a.h.x][a.h.y].append(*it,a.h.r);
        }
      }
    }

    void clear() {
      ADS_set tmp;
      swap(tmp);
    }
    size_type erase(const key_type &k) {
      Hash h{k};
      auto rc {entries[h.x][h.y].erase(k,h.r)};
      curr_size -= rc;
      return rc;
    }

    size_type count(const key_type &k) const {
      return Address{k, this} ? 1 : 0;
    }
    iterator find(const key_type &k) const {
      if (Address a{k,this}) return {entries, a};
      return end();
    }

    void swap(ADS_set &other) {
      using std::swap;
      swap(entries, other.entries);
      swap(curr_size, other.curr_size);
    }

    const_iterator begin() const {
      return const_iterator{entries};
    }
    const_iterator end() const {
      return const_iterator{};
    }

    void dump(std::ostream &o = std::cerr) const {
      o << "BISEH<" << typeid(Key).name() << ',' << N << ">, NX = " << NX << ", NY = " << NY << ", curr_size = " << curr_size << "\n";
      o << "sizeof(Hash)=" << sizeof(Hash) << ", sizeof(Address)=" << sizeof(Address) << "\n\n";
      for (size_t x{0}; x < NX; ++x) {
        o << "Bereich " << x << "\n";
        for (size_t y{0}; y < NY; ++y) {
          o << y << ": nz=" << entries[x][y].nz;
          for (size_t z{0}; z < entries[x][y].nz; ++z) {
            o << " [n=" << entries[x][y].blocks[z].n << "| ";
            for (size_t i{0}; i < entries[x][y].blocks[z].n; ++i) {
              o << entries[x][y].blocks[z].values[i] << ' ';
            }
            o << "]";
          }
          o << "\n";
        }
        o << "\n";
      }
    }

    friend bool operator==(const ADS_set &lhs, const ADS_set &rhs) {
      if (lhs.curr_size != rhs.curr_size) return false;
      for (const auto &k: rhs) if (!lhs.count(k)) return false; // nicht bei B+-Baum wegen Ordnung!
      return true;
    }
    friend bool operator!=(const ADS_set &lhs, const ADS_set &rhs) {
      return !(lhs == rhs);
    }
};

template <typename Key, size_t N>
struct ADS_set<Key,N>::Hash {
    size_t x:XBITS, y:YBITS, r:std::numeric_limits<size_t>::digits-XBITS-YBITS;
    Hash(const key_type &k) {
      auto h{hasher{}(k)};
      x =  h%NX          /* h&(NX-1)          */;
      y =  h/NX%NY       /* (h>>XBITS)&(NY-1) */;
      r =  h/(NX*NY)     /* h>>(XBITS+YBITS)  */;
    }
};
template <typename Key, size_t N>
struct ADS_set<Key,N>::Address {
    Hash h;
    const key_type *p;
    Address(Hash h, const key_type *p): h{h}, p{p} {}
    Address(const key_type &k, const ADS_set *s): h{Hash{k}}, p{s->entries[h.x][h.y].find(k,h.r)} {}
    operator bool() const {
      return p != nullptr;
    }
};
template <typename Key, size_t N>
struct ADS_set<Key,N>::Block {
    size_t n{0};
    key_type values[N];
    const key_type *append(const key_type &k) {
      return n == N ? nullptr : &(values[n++] = k);
    }
    const key_type *find(const key_type &k) const {
      for (size_t i {0}; i < n; ++i)
        if (key_equal{}(k, values[i])) return &values[i]; // values+i
      return nullptr;
    }
    size_t erase(const key_type &k) {
      for (size_t i{0}; i < n; ++i) {
        if (key_equal{}(k,values[i])) {
          if (i != --n) values[i] = values[n];
          return 1;
        }
      }
      return 0;
    }
};
template <typename Key, size_t N>
struct ADS_set<Key,N>::Entry {
    size_t nz{0};
    Block *blocks{nullptr};
    ~Entry() {
      delete[] blocks;
    }
    const key_type *append(const key_type &k, size_t r) {
      if (!blocks) blocks = new Block[nz=1];
      while (true) {
        if (auto rc {blocks[r%nz].append(k)}) return rc;
        resize();
      }
    }
    void resize() {
      size_t new_nz {2*nz};
      Block *new_blocks {new Block[new_nz]};
      for (size_t z{0}; z < nz; ++z)
        for (size_t i{0}; i < blocks[z].n; ++i)
          new_blocks[Hash{blocks[z].values[i]}.r%new_nz].append(blocks[z].values[i]);
      delete[] blocks;
      blocks = new_blocks;
      nz = new_nz;
    }
    const key_type *find(const key_type &k, size_t r) const {
      return blocks ? blocks[r%nz].find(k): nullptr;
    }
    size_t erase(const key_type &k, size_t r) {
      return blocks ? blocks[r%nz].erase(k) : 0;
    }
};

template <typename Key, size_t N>
class ADS_set<Key,N>::Iterator {
public:
    using value_type = Key;
    using difference_type = std::ptrdiff_t;
    using reference = const value_type &;
    using pointer = const value_type *;
    using iterator_category = std::forward_iterator_tag;
private:
    Entry **e;
    size_t x, y, z;
    pointer p;
    void skip() {
      for (; x < NX; ++x) {
        for (; y < NY; ++y) {
          for (; z < e[x][y].nz; ++z) {
            if (!p) p = e[x][y].blocks[z].values;
            if (p && p < e[x][y].blocks[z].values + e[x][y].blocks[z].n) return;
            p = nullptr;
          }
          z = 0;
        }
        y = 0;
      }
    }
public:
    explicit Iterator(Entry **e): e{e}, x{0}, y{0}, z{0}, p{nullptr} {
      skip();
    }
    Iterator(Entry **e, const Address &a): e{e}, x{a.h.x}, y{a.h.y}, z{a.h.r%e[x][y].nz}, p{a.p} {
    }
    Iterator(): e{nullptr}, x{0}, y{0}, z{0}, p{nullptr} {
    }
    reference operator*() const {
      return *p;
    }
    pointer operator->() const {
      return p;
    }
    Iterator &operator++() {
      ++p;
      skip();
      return *this;
    }
    Iterator operator++(int) {
      auto rc{*this};
      ++*this;
      return rc;
    }
    friend bool operator==(const Iterator &lhs, const Iterator &rhs) {
      return lhs.p == rhs.p;
    }
    friend bool operator!=(const Iterator &lhs, const Iterator &rhs) {
      return !(lhs == rhs);
    }
};

template <typename Key, size_t N> void swap(ADS_set<Key,N> &lhs, ADS_set<Key,N> &rhs) { lhs.swap(rhs); }

#endif // ADS_SET_H