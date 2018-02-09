#ifndef ARRAY_H
#define ARRAY_H

#include <stdint.h>
#include <std/memory.h>
#include <std/Hash.h>

#define init_array(Type)    \
    typedef struct Array_##Type {   \
        Type *data; \
        uint32_t size;  \
        uint32_t cap;   \
    } Array_##Type; \
    \
    static inline Array_##Type array_##Type##_new() {   \
        return (Array_##Type) { (Type*) calloc(16, sizeof(Type)), 0, 16 };  \
    }   \
    \
    static inline uint32_t array_##Type##_hash(const Array_##Type *self) {  \
        return murmur(self->data, self->size*sizeof(Type)); \
    }   \
    \
    static inline void array_##Type##_free(Array_##Type *self) {    \
        free(self->data);   \
        self->data = 0; \
        self->size = 0; \
        self->cap = 0;  \
    }   \
    \
    static inline Array_##Type array_##Type##_copy(const Array_##Type *other) { \
        Array_##Type arr = { (Type*) calloc(other->cap, sizeof(Type)), other->size, other->cap };   \
        copy(arr.data, other->data, other->size*sizeof(Type));    \
        return arr; \
    }   \
    \
    void array_##Type##_expand(Array_##Type *self) {    \
        Type *old = self->data; \
        self->cap <<= 1;    \
        self->data = (Type*) calloc(self->cap, sizeof(Type));   \
        copy(self->data, old, self->size*sizeof(Type));   \
        free(old);  \
    }   \
    \
    static inline void array_##Type##_push(Array_##Type *self, const Type item) { \
        self->data[self->size++] = item;    \
        if(self->size == self->cap) array_##Type##_expand(self);    \
    }   \
        
#define array_foreach(Type, array, lambda) {    \
    for(uint32_t _i=0; _i<(array).size; ++_i) {   \
        lambda((array).data[_i]);   \
    }   \
}   \
    
#endif
