#ifndef LANDMARKS_LANDMARK_TYPES_H
#define LANDMARKS_LANDMARK_TYPES_H

#include "../utilities.h"

#include <utility>
#include <ext/hash_set>
#include <functional>

class hash_pointer {
public:
    size_t operator()(int *p) {
    //size_t operator()(const void *p) const {
      return std::hash<int *>()(p);
    }
    //size_t operator()(const void *p) {
    //size_t operator()(const void *p) const {
    //  return __gnu_cxx::hash<const void *>()(p);
    //}
};


typedef __gnu_cxx::hash_set<std::pair<int, int>, hash_int_pair> lm_set;
#endif
