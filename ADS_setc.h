#ifndef ADS_SET_H
#define ADS_SET_H

#include <functional>
#include <algorithm>
#include <iostream>
#include <stdexcept>


template <typename Key, size_t N = 100>
class ADS_set {
public:
    class Iterator;
    using value_type = Key;
    using key_type = Key;
    using reference = value_type &;
    using const_reference = const value_type &;
    using size_type = size_t;
    using difference_type = std::ptrdiff_t;
    //using const_iterator = /* iterator type */;
    //using iterator = const_iterator;
    //using key_compare = std::less<key_type>;                         // B+-Tree
    using key_equal = std::equal_to<key_type>;                       // Hashing
    using hasher = std::hash<key_type>;                              // Hashing

private:
    struct Bucket;

    size_type d {0}; //globale tiefe, bits, auf die geschaut wird
    Bucket **bucketsit{nullptr};
    size_type c_sz {0}; //vorhandene keys insg.

    const key_type *append(const key_type &k) {
      std::cout << d << std::endl;
      //if (!bucketsit) bucketsit = new Bucket[d = 1];
      while (true) {
        auto rc {bucketsit[h(k)]->b_append(k)}; // bucketsit[h(k)%d] ???
        if (rc) return rc;
        if (expow((d - bucketsit[h(k)]->t)) != 1) split(k);
        else {resize(); split(k); }
      }
    }

    void resize() {
      size_type new_d {d + 1};
      //size_type e_d {expow(new_d)};
      Bucket **new_bucketsit {new Bucket*[expow(new_d)]}; //std::cout << expow(new_d) << "\n";
      //for (size_type z {expow(new_d)/2}; z < expow(new_d); ++z) {
      for (size_type i {0}; i < expow(d); ++i) {
        new_bucketsit[i] = bucketsit[i]; //0518 std::swap(new_bucketsit[i], bucketsit[i]); //new_bucketsit[(hasher{}(bucketsit[z].values[i]) & ((1 << new_d) - 1))].b_append(bucketsit[z].values[i]);
        new_bucketsit[(i + (expow(new_d)/2))] = bucketsit[i]; //0518 = new_bucketsit[i]; //neuer pointer zeigt auch darauf
      }
      //}
      for (size_type a {0}; a < expow(d); ++a) {
        bucketsit[a] = nullptr;
        delete bucketsit[a];

      }
      delete[] bucketsit;
      bucketsit = new_bucketsit;
      d = new_d;
    }

    void split(const key_type &k) {
      size_type b1;
      size_type b2 {h(k)};
      if (b2 < expow(d)/2) b1 = b2 + (expow(d)/2);
      else b1 = b2 - (expow(d)/2);
      for (size_type b{0}; b < expow(d); ++b) {
        if ((bucketsit[b2] == bucketsit[b]) && (b != b2)) {
          b1 = b;
          break;
        }
      }
      bucketsit[b2] = new Bucket();
      for (size_type a{0}; a < N; ++a) { //bucketsit[b1]->n
        if (h(bucketsit[b1]->values[a]) == b2) {
          bucketsit[b2]->b_append(bucketsit[b1]->values[a]);
          bucketsit[b1]->remove(bucketsit[b1]->values[a]);
        }
      }
      bucketsit[b1]->t += 1;
      bucketsit[b1]->dt += 1;
      bucketsit[b2]->t = bucketsit[b1]->t;
      bucketsit[b2]->dt = bucketsit[b1]->t;
    }

    size_type h(const key_type &key) const {
      return (hasher{}(key) & ((1 << d) - 1));
    }

    static size_type expow(const size_type& powpow) {
      return (1 << powpow);
    }

public:
    ADS_set() : d{0}, bucketsit{new Bucket*[expow(d)]}, c_sz{0} {                    // PH1
      //std::cout << expow(d) << "\n";
      for (size_type x{0}; x < expow(d); ++x) bucketsit[x] = new Bucket();
    }

    ADS_set(std::initializer_list<key_type> ilist) : ADS_set{} {                     // PH1
      insert(ilist);
    }

    template<typename InputIt> ADS_set(InputIt first, InputIt last) : ADS_set{} {    // PH1
      insert(first, last);
    }

    ~ADS_set() {
      for (size_t x{0}; x < expow(d); ++x) {
        if (expow(d - bucketsit[x]->t) != 1) {
          bucketsit[x]->t += 1;
          bucketsit[x]->dt += 1;
          bucketsit[x] = nullptr;
        }
        else {
          delete[] bucketsit[x];
        }
      }
      delete[] bucketsit;
    }

    size_type size() const {                                                         // PH1
      return c_sz;
    }

    bool empty() const {                                                             // PH1
      if (c_sz == 0) return true;
      return false;
    }

    void insert(std::initializer_list<key_type> ilist) {                             // PH1
      insert(std::begin(ilist),std::end(ilist));
    }                 // PH1

    template<typename InputIt> void insert(InputIt first, InputIt last) {
      for (auto it {first}; it != last; ++it) {
        if (!(count(*it))) {
          ++c_sz;
          append(*it);
        }

      }
    }  // PH1

    size_type count(const key_type &key) const {
      if (bucketsit[h(key)]->find(key) != nullptr) return 1;
      return 0;
    }                          // PH1

    void dump(std::ostream &o = std::cerr) const {
      o << "Extendible Hashing<" << typeid(Key).name() << ',' << N << ">, g.Tiefe : " << d << ", akt.Größe: " << c_sz << "\n";
      size_t bkt_cnt {0};
      for (size_type a {0}; a < expow(d); ++a) {
        if (expow(d - bucketsit[a]->dt) != 1) {
          bucketsit[a]->dt += 1;
          continue;
        }
        bkt_cnt++;
        o << "Bucket " << a << ", akt.BGröße: " << bucketsit[a]->n << ", l.Tiefe: " << bucketsit[a]->t << " | ";
        for (size_type b {0}; b < bucketsit[a]->n; ++b) {
          o << hasher{}(bucketsit[a]->values[b]) << " ";
        }
        bucketsit[a]->dt = bucketsit[a]->t;
        o << ";" << std::endl;
        bucketsit[a]->n = -1;
      }
      std::cout << bkt_cnt << std::endl;
    }

};

template <typename Key, size_t N>
struct ADS_set<Key,N>::Bucket {
    size_type n {0};
    key_type values[N];
    size_type t {0};
    size_type dt{0};

    const key_type *b_append(const key_type &k) {
      if (n == N) return nullptr;
      return &(values[n++] = k);
    }

    const key_type *find(const key_type &k) const {
      for (size_type i {0}; i < n; ++i) {
        if (key_equal{}(k, values[i])) return &values[i];
      }
      return nullptr;
    }

    size_type remove(const key_type &k) {
      //if (n >= N) return 0;
      for (size_t i{0}; i < n; ++i) {
        if (key_equal{}(k,values[i])) {
          if (i != --n) values[i] = values[n];
          return 1;
        }
      }
      return 0;
    }
};

#endif // ADS_SET_H