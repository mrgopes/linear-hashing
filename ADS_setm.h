#ifndef ADS_SET_H
#define ADS_SET_H

#include <functional>
#include <algorithm>
#include <iostream>
#include <stdexcept>

template <typename Key, size_t N = 7>
class ADS_set {
public:
    //class /* iterator type (implementation-defined) */;
    using value_type = Key;
    using key_type = Key;
    using reference = value_type &;
    using const_reference = const value_type &;
    using size_type = size_t;
    using difference_type = std::ptrdiff_t;
    //using const_iterator = /* iterator type */;
    //using iterator = const_iterator;
    using key_equal = std::equal_to<key_type>;                       // Hashing
    using hasher = std::hash<key_type>;                              // Hashing

private:
    size_type d; //globale Tiefe
    struct Bucket;
    Bucket **directory {nullptr}; //Array von Pointer
    size_type current; //Anzahl Elemente

    void append(const key_type &key) {
      size_type idx {h(key)};
      Bucket *target = directory[idx];
      if(!count(key)){
        if(target->is_full()) {
          if(target->t<d){
            split(target);
            append(key);
            return;
          }
          if(target->t==d){
            resize();
            append(key);
            return;
          }
        }
        target->add_e(key);
      }
    }

    void resize(){
      size_type new_d {d+1};
      Bucket **new_directory = new Bucket*[1<<new_d];
      for(int i{0}; i<(1<<d); ++i){
        new_directory[i] = directory[i];
        new_directory[i+(1<<d)]= new_directory[i];
      }
      delete[] directory;
      directory=new_directory;
      d=new_d;
    }

    void split(Bucket* target){
      Bucket* new_b1 = new Bucket;
      Bucket* new_b2 = new Bucket;
      for(size_type i{0}; i<N; ++i){
        size_type idx = h(target->values[i]);
        if(idx & ((1<<target->t)+1)) new_b1->add_e(target->values[i]);
        else new_b2->add_e(target->values[i]);
      }
      //size_type idx = h(target->values[0]);
      for(int i{0}; i<(1<<d); ++i){
        if(directory[i]==target){
          if(i & (((1<<target->t)))) {
            directory[i]=new_b1;
          }
          else {
            directory[i]=new_b2;
          }
        }
      }
      new_b1->t=target->t+1;
      new_b2->t=target->t+1;
      delete target;
      target=nullptr;
    }

    size_type h(const key_type &key) const {return hasher{}(key) & ((1<<d)-1);}

public:
    ADS_set(): d{0}, directory{new Bucket*[(1<<d)]}, current{0}{
      directory[0]= new Bucket;
    }                    // PH1
    ADS_set(std::initializer_list<key_type> ilist):ADS_set{}{
      insert(ilist);
    }       // PH1
    template<typename InputIt> ADS_set(InputIt first, InputIt last):ADS_set {} {insert(first, last);}     // PH1
    //ADS_set(const ADS_set &other);

    //~ADS_set();

    //ADS_set &operator=(const ADS_set &other);
    //ADS_set &operator=(std::initializer_list<key_type> ilist);

    size_type size() const {return current;}                                              // PH1
    bool empty() const {
      if(size()==0) return true;
      return false;
    }                                                 // PH1

    void insert(std::initializer_list<key_type> ilist){
      return insert(std::begin(ilist), std::end(ilist));
    }                  // PH1
    //std::pair<iterator,bool> insert(const key_type &key);
    template<typename InputIt> void insert(InputIt first, InputIt last){
      for(auto it=first; it!=last; ++it){
        if(!count(*it)){
          ++current;
          append(*it);
        }
      }
    } // PH1

    //void clear();
    //size_type erase(const key_type &key);

    size_type count(const key_type &key) const {
      if(directory[h(key)]->find(key) != nullptr) return 1;
      return 0;
    }                          // PH1
    //iterator find(const key_type &key) const;

    //void swap(ADS_set &other);

    //const_iterator begin() const;
    //const_iterator end() const;

    void dump(std::ostream &o = std::cerr) const{
      o << "Extendible Hashing" << typeid(Key).name() << ',' << N << " , d=" << d << ", curr_size=" << current << "\n";
    }
    //friend bool operator==(const ADS_set &lhs, const ADS_set &rhs);
    //friend bool operator!=(const ADS_set &lhs, const ADS_set &rhs);
};

template <typename Key, size_t N>
struct ADS_set<Key,N>::Bucket{
    size_type t {0}; //lokale Tiefe
    key_type values[N]; //Entries
    size_type n{0}; //Anzahl Entries aktuell

    void add_e(const key_type &key){
      if(!is_full()) values[n++] = key;
    }
    bool is_full(){
      return n==N;
    }
    const key_type *find(const key_type &key) const{
      for(size_t i{0}; i<n; ++i){
        if(key_equal{}(key, values[i])) return &values[i];
      }
      return nullptr;
    }
};

#endif // ADS_SET_H