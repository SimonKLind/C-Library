#ifndef HASHSET_H
#define HASHSET_H

#include <stdint.h>
#include <std/memory.h>
#include <std/Hash.h>

#ifndef LOAD_FACTOR
    #define LOAD_FACTOR 0.9
#endif

#define init_hashset(Type, equals, hash)    \
    typedef struct HashSet_##Type##_Node {  \
        Type item;  \
        uint32_t hash;  \
        uint16_t active;    \
        uint16_t alive; \
    } HashSet_##Type##_Node;    \
    \
    typedef struct HashSet_##Type { \
        HashSet_##Type##_Node *nodes;   \
        uint32_t size;  \
        uint32_t cap;   \
    } HashSet_##Type;   \
    \
    static inline HashSet_##Type hashset_##Type##_new() {   \
        return (HashSet_##Type) { (HashSet_##Type##_Node*) calloc(16, sizeof(HashSet_##Type##_Node)), 0, 16 };  \
    }   \
    \
    static inline HashSet_##Type hashset_##Type##_copy(const HashSet_##Type *other) {    \
        HashSet_##Type set = { (HashSet_##Type##_Node*) calloc(other->cap, sizeof(HashSet_##Type##_Node)), other->size, other->cap };   \
        copy(set.nodes, other->nodes, other->cap*sizeof(HashSet_##Type##_Node)); \
        return set; \
    }   \
    \
    void hashset_##Type##_expand(HashSet_##Type *self) {    \
        HashSet_##Type##_Node *old = self->nodes;   \
        const uint32_t old_cap = self->cap; \
        self->cap <<= 1;    \
        self->nodes = (HashSet_##Type##_Node*) calloc(self->cap, sizeof(HashSet_##Type##_Node));    \
        const uint32_t mask = self->cap-1;  \
        HashSet_##Type##_Node temp; \
        for(uint32_t i=0, index; i<old_cap; ++i) {  \
            if(!old[i].active) continue;    \
            index = old[i].hash&mask;   \
            for(uint32_t dist = 0; self->nodes[index].active; ++dist) { \
                if(index-(self->nodes[index].hash&mask)+self->cap*(index-(self->nodes[index].hash&mask)>=self->cap) < dist) {   \
                    temp = self->nodes[index];  \
                    self->nodes[index] = old[i];    \
                    old[i] = temp;  \
                }   \
                index = (index+1)&mask; \
            }   \
            self->nodes[index] = old[i];    \
        }   \
        free(old);  \
    }   \
    \
    void hashset_##Type##_put(HashSet_##Type *self, const Type item) {  \
        const uint32_t mask = self->cap-1;  \
        const uint32_t hash = hash(item);   \
        uint32_t index = hash&mask; \
        HashSet_##Type##_Node current = { item, hash, 1, 1 }, temp;  \
        for(uint32_t dist = 0; self->nodes[index].active; ++dist) { \
            if(self->nodes[index].hash == hash && equals(self->nodes[index].item, item)) return;    \
            else if(index-(self->nodes[index].hash&mask)+self->cap*(index-(self->nodes[index].hash&mask)>=self->cap) < dist) {  \
                temp = self->nodes[index];  \
                self->nodes[index] = current;   \
                current = temp; \
            }   \
            index = (index+1)&mask; \
        }   \
        self->nodes[index] = current;   \
        if(++self->size >= LOAD_FACTOR*self->cap) hashset_##Type##_expand(self);    \
    }   \
    \
    uint8_t hashset_##Type##_contains(const HashSet_##Type *self, const Type item) {    \
        const uint32_t hash = hash(item);   \
        const uint32_t mask = self->cap-1;  \
        for(uint32_t index = hash&mask, dist = 0;   \
            self->nodes[index].alive && index-(self->nodes[index].hash&mask)+self->cap*(index-(self->nodes[index].hash&mask)>=self->cap) >= dist;   \
            index = (index+1)&mask, ++dist) {   \
            if(self->nodes[index].active && self->nodes[index].hash == hash && equals(self->nodes[index].item, item)) return 1; \
        }   \
        return 0;   \
    }   \
    \
    uint8_t hashset_##Type##_remove(HashSet_##Type *self, const Type item) {  \
        const uint32_t hash = hash(item);   \
        const uint32_t mask = self->cap-1;  \
        for(uint32_t index = hash&mask, dist = 0;   \
            self->nodes[index].alive && index-(self->nodes[index].hash&mask)+self->cap*(index-(self->nodes[index].hash&mask)>=self->cap) >= dist;   \
            index = (index+1)&mask, ++dist) {   \
            if(self->nodes[index].active && self->nodes[index].hash == hash && equals(self->nodes[index].item, item)) { \
                self->nodes[index].active = 0;  \
                --self->size;   \
                return 1;   \
            }   \
        }   \
        return 0;   \
    }   \
    \
    static inline void hashset_##Type##_to_array(const HashSet_##Type *self, Type *arr) {   \
        for(uint32_t i=0; i<self->cap; ++i) {   \
            if(!self->nodes[i].active) continue;    \
            *arr++ = self->nodes[i].item;    \
        }   \
    }   \
    \
    static inline void hashset_##Type##_free(HashSet_##Type *self) {    \
        free(self->nodes);  \
    }   \

#define hashset_foreach(Type, set, lambda) {    \
    for(uint32_t _i = 0; _i<(set).cap; ++_i) {  \
        if(!(set).nodes[_i].active) continue;   \
        lambda((set).nodes[_i].item);   \
    }   \
}   \

#endif
