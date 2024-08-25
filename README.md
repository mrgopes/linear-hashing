# Linear Hashing

This is an implementation of the Linear Hashing algorithm for the course Algorithms and Data Structures 1 at the University of Vienna in the Summer Semester 2024.
Libraries used for testing etc., that were provided by the professors and were not meant to appear in a repository, were purged and are not included.
These are for example Simpletest or Btest.

## File description

**ADS_ph1.h** is the implementation sufficient for the first submission. \
**ADS_set.h** is the full implementation that was considered at the end for evaluation with all of the required functionality. \
**main.cpp** is a very simple program that I used for testing and that I have overwritten many times.
<br><br>
The Abschlussklausur implementation (a reverse mode for the iterator) is not included.

## Statistics

	

               703,967,569 calls to std::equal_to<ADS_set::key_type>::operator()
                64,147,035 calls to std::hash<ADS_set::key_type>::operator()
                   149,222 calls to new allocating 143,227,576 bytes in total (*)
                       600 calls to new[] allocating 2,229,248 bytes in total
                85,678,824 bytes dynamic memory peak

    (*) including 27 calls by the unit test to create ADS_set objects

## Performance ratings

Bewertung Zeit	

1.02

Bewertung Speicher	

0.74

## Improvement suggestions

Improvements in the performance of the implementation could possibly be achieved through a single pointer array (merging the overflow pointer array and the pointer array for the current values).
