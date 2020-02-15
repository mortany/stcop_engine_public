#pragma once
#include <vector>
#include "xalloc.h"

#define DEF_VECTOR(N, T)\
    typedef xr_vector<T> N;\
    typedef N::iterator N##_it;

#define DEFINE_VECTOR(T, N, I)\
    typedef xr_vector<T> N;\
    typedef N::iterator I;

// vector
template <typename T, typename allocator = xalloc<T> >
using xr_vector = class std::vector<T, allocator>;