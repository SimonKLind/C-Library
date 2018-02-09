#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H

#include <std/memory.h>
#include <std/Hash.h>
#include <stdint.h>

#define init_heap(Type, comp_func)  \
    typedef struct Heap_##Type {    \
        Type *data; \
        uint32_t size;  \
        uint32_t cap;   \
    } Heap_##Type;  \
    \
    static inline Heap_##Type heap_##Type##_new() { \
        return (Heap_##Type) { (Type*) calloc(16, sizeof(Type)), 0, 16 };   \
    }   \
    \
    static inline Heap_##Type heap_##Type##_copy(const Heap_##Type *other) {    \
        Heap_##Type heap = { (Type*) calloc(other->cap, sizeof(Type)), other->size, other->cap };   \
        copy(heap.data, other->data, heap.size*sizeof(Type));   \
        return heap;    \
    }   \
    \
    void heap_##Type##_float(Heap_##Type *self, uint32_t heap_index) {  \
        Type temp;  \
        uint32_t above = heap_index>>1; \
        while(above >= 1 && comp_func(self->data[heap_index-1], self->data[above-1]) < 0) { \
            temp = self->data[heap_index-1];    \
            self->data[heap_index-1] = self->data[above-1]; \
            self->data[above-1] = temp; \
            heap_index >>= 1;   \
            above >>= 1;    \
        }   \
    }   \
    \
    void heap_##Type##_sink(Heap_##Type *self, uint32_t heap_index) {   \
        Type temp;  \
        uint32_t swap_index = heap_index << 1;  \
        while(swap_index <= self->size) {   \
            if(swap_index < self->size && comp_func(self->data[swap_index], self->data[swap_index-1]) < 0) ++swap_index;    \
            if(comp_func(self->data[heap_index-1], self->data[swap_index-1]) <= 0) break;   \
            temp = self->data[swap_index-1];    \
            self->data[swap_index-1] = self->data[heap_index-1];    \
            self->data[heap_index-1] = temp;    \
            heap_index = swap_index;    \
            swap_index <<= 1;   \
        }   \
    }   \
    \
    static inline void heap_##Type##_reheap(Heap_##Type *self) {    \
        for(uint32_t i=self->size>>1; i; --i) heap_##Type##_sink(self, i); \
    }   \
    \
    Heap_##Type heap_##Type##_from_array(const Type *array, const uint32_t len) {   \
        uint32_t cap = 16;  \
        while(cap <= len) cap <<= 1;    \
        Heap_##Type heap = { (Type*) calloc(cap, sizeof(Type)), len, cap }; \
        copy(heap.data, array, len*sizeof(Type));   \
        for(uint32_t i=len>>1; i; --i) heap_##Type##_sink(&heap, i);    \
        return heap;    \
    }   \
    \
    static inline uint32_t heap_##Type##_hash(const Heap_##Type *self) {    \
        return murmur(self->data, self->size*sizeof(Type)); \
    }   \
    \
    void heap_##Type##_expand(Heap_##Type *self) {  \
        Type *old = self->data; \
        self->cap <<= 1;    \
        self->data = (Type*) calloc(self->cap, sizeof(Type));   \
        copy(self->data, old, self->size*sizeof(Type)); \
        free(old);  \
    }   \
    \
    static inline void heap_##Type##_push(Heap_##Type *self, const Type item) { \
        self->data[self->size++] = item;    \
        heap_##Type##_float(self, self->size);  \
        if(self->size == self->cap) heap_##Type##_expand(self); \
    }   \
    \
    uint8_t heap_##Type##_pop(Heap_##Type *self, Type *out) {   \
        switch(self->size) {    \
            case 0: \
                return 0;   \
            case 1: \
                *out = self->data[0];   \
                self->size = 0; \
                return 1;   \
            default:    \
                *out = self->data[0];   \
                self->data[0] = self->data[--self->size];   \
                heap_##Type##_sink(self, 1);    \
                return 1;   \
        }   \
    }   \
    \
    static inline void heap_##Type##_peek(const Heap_##Type *self, Type *out) {    \
        if(self->size > 0) *out = self->data[0];    \
    }   \
    \
    static inline void heap_##Type##_free(Heap_##Type *self) {  \
        free(self->data);   \
        self->data = 0; \
        self->cap = self->size = 0; \
    }   \

#define heap_foreach(Type, heap, lambda) {  \
    for(uint32_t _i=0; _i<(heap).size; ++_i) { \
        lambda((heap).data[_i]); \
    }   \
}   \

#endif
