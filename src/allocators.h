#ifndef _ALLOCATOR_H_
#define _ALLOCATOR_H_

#include "types.h"

struct Allocator {
    void* (*alloc) (size_t);
    void  (*free)  (void*);
};

#endif //_ALLOCATOR_H_