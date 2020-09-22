#ifndef __BINLEP_STL_INTERNAL_ALLOC_H
#define __BINLEP_STL_INTERNAL_ALLOC_H


#include <cstdio>
#include <cstdlib>
#include "stl_config.h"

__STL_BEGIN_NAMESPACE

template <int _inst>
class __malloc_alloc_template{

public:
    static void* allocate(size_t __n){
        void* __result = malloc(__n);
        if(!__result){
            fprintf(stderr, "out of memory\n");
            exit(1);
        }
        return __result;
    }

    static void deallocate(void* __p, size_t /* __n */){
        free(__p);
    }

    static void* rellocate(void* __p, size_t /* __old_sz */, size_t __new_sz){
        void* __result = realloc(__p, __new_sz);
        if(!__result){
            fprintf(stderr, "out of memory\n");
            exit(1);
        }
        return __result;
    }
};

typedef __malloc_alloc_template<0> malloc_alloc;

typedef malloc_alloc alloc;


__STL_END_NAMESPACE

#endif // __BINLEP_STL_INTERNAL_ALLOC_H