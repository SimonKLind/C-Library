#ifndef HASHSET_H
#define HASHSET_H

#include <stdint.h>
#include <std/memory.h>
#include <std/Hash.h>

#ifndef RESIZE_THRESHOLD
    #define RESIZE_THRESHOLD 0.75
#endif

#define init_hashset(Type, equals, hash)    \
    typedef struct HashSet_##Type##_Node {  \
        Type item;  \
        struct HashSet_##Type##_Node *next; \
    } HashSet_##Type##_Node;    \
    \
    typedef struct HashSet_##Type { \
        HashSet_##Type##_Node **buckets;    \
        uint32_t size;  \
        uint32_t num_buckets;   \
    } HashSet_##Type;   \
    \
    static inline HashSet_##Type hashset_##Type##_new() {   \
        return (HashSet_##Type) { (HashSet_##Type##_Node**) calloc(16, sizeof(HashSet_##Type##_Node*)), 0, 16 };    \
    }   \
    \
    HashSet_##Type hashset_##Type##_copy(const HashSet_##Type *other) { \
        HashSet_##Type set = (HashSet_##Type) { \
            (HashSet_##Type##_Node**) calloc(other->num_buckets, sizeof(HashSet_##Type##_Node*)),   \
            other->size,    \
            other->num_buckets  \
        };  \
        HashSet_##Type##_Node *new_temp;    \
        for(uint32_t i=0; i<other->num_buckets; ++i) {  \
            for(HashSet_##Type##_Node *temp = other->buckets[i]; temp; temp = temp->next) { \
                new_temp = (HashSet_##Type##_Node*) malloc(sizeof(HashSet_##Type##_Node));  \
                *new_temp = (HashSet_##Type##_Node) { temp->item, set.buckets[i] };  \
                set.buckets[i] = new_temp;  \
            }   \
        }   \
        return set; \
    }   \
    \
    void hashset_##Type##_expand(HashSet_##Type *self) {    \
        HashSet_##Type##_Node **old_buckets = self->buckets;    \
        const uint32_t old_num_buckets = self->num_buckets; \
        self->num_buckets <<= 1;    \
        self->buckets = (HashSet_##Type##_Node**) calloc(self->num_buckets, sizeof(HashSet_##Type##_Node*));    \
        HashSet_##Type##_Node *temp;    \
        for(uint32_t i=0, index; i<old_num_buckets; ++i) {  \
            while(old_buckets[i]) { \
                index = hash(old_buckets[i]->item)&(self->num_buckets-1);   \
                temp = old_buckets[i];  \
                old_buckets[i] = old_buckets[i]->next;  \
                temp->next = self->buckets[index];  \
                self->buckets[index] = temp;    \
            }   \
        }   \
        free(old_buckets);  \
    }   \
    \
    void hashset_##Type##_put(HashSet_##Type *self, Type item) {    \
        const uint32_t index = hash(item)&(self->num_buckets-1);    \
        HashSet_##Type##_Node *temp = self->buckets[index]; \
        while(temp) {   \
            if(equals(temp->item, item)) return;  \
            temp = temp->next;  \
        }   \
        temp = self->buckets[index];    \
        self->buckets[index] = (HashSet_##Type##_Node*) malloc(sizeof(HashSet_##Type##_Node));  \
        *self->buckets[index] = (HashSet_##Type##_Node) { item, temp }; \
        if(++self->size >= RESIZE_THRESHOLD*self->num_buckets) hashset_##Type##_expand(self);   \
    }   \
    \
    uint8_t hashset_##Type##_remove(HashSet_##Type *self, Type item) {  \
        const uint32_t index = hash(item)&(self->num_buckets-1);    \
        if(!self->buckets[index]) return 0; \
        if(equals(self->buckets[index]->item, item)) {  \
            HashSet_##Type##_Node *temp = self->buckets[index]; \
            self->buckets[index] = self->buckets[index]->next;  \
            free(temp); \
            --self->size;   \
            return 1;   \
        }   \
        for(HashSet_##Type##_Node *temp = self->buckets[index], *next = self->buckets[index]->next; next; temp = next, next = next->next) { \
            if(equals(next->item, item)) {  \
                temp->next = next->next;    \
                free(next); \
                --self->size;   \
                return 1;   \
            }   \
        }   \
        return 0;   \
    }   \
    \
    uint8_t hashset_##Type##_contains(const HashSet_##Type *self, Type item) {  \
        const uint32_t index = hash(item)&(self->num_buckets-1);    \
        for(HashSet_##Type##_Node *temp = self->buckets[index]; temp; temp = temp->next) {  \
            if(equals(temp->item, item)) return 1;  \
        }   \
        return 0;   \
    }   \
    \
    void hashset_##Type##_to_array(const HashSet_##Type *self, Type *arr) { \
        if(!arr) return;    \
        for(uint32_t i=0; i<self->num_buckets; ++i) {   \
            for(HashSet_##Type##_Node *temp = self->buckets[i]; temp; temp = temp->next) {  \
                *arr++ = temp->item;    \
            }   \
        }   \
    }   \
        \
    void hashset_##Type##_free(HashSet_##Type *self) {  \
        for(uint32_t i=0; i<self->num_buckets; ++i) {   \
            for(HashSet_##Type##_Node *temp = self->buckets[i]; temp; temp = self->buckets[i]) {    \
                self->buckets[i] = self->buckets[i]->next;  \
                free(temp); \
            }   \
        }   \
        free(self->buckets);    \
        *self = (HashSet_##Type) { 0 };   \
    }   \

#define hashset_foreach(Type, set, lambda) {    \
    for(uint32_t _i = 0; _i < (set).num_buckets; ++_i) {    \
        for(HashSet_##Type##_Node *_temp = (set).buckets[_i]; _temp; _temp = _temp->next) {  \
            lambda(_temp->item);    \
        }   \
    }   \
}   \

#endif
