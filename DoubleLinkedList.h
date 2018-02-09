#ifndef DOUBLE_LINKED_LIST_H
#define DOUBLE_LINKED_LIST_H

#include <std/memory.h>
#include <std/Hash.h>
#include <stdint.h>

#define init_bilist(Type)   \
    typedef struct BiList_##Type##_Node {   \
        Type item;  \
        struct BiList_##Type##_Node *next;  \
        struct BiList_##Type##_Node *prev;  \
    } BiList_##Type##_Node; \
    \
    typedef struct BiList_##Type {  \
        BiList_##Type##_Node *first;    \
        BiList_##Type##_Node *last; \
        uint32_t size;  \
    } BiList_##Type;    \
    \
    static inline BiList_##Type bilist_##Type##_new() { \
        return (BiList_##Type) { 0, 0, 0 }; \
    }   \
    \
    BiList_##Type bilist_##Type##_copy(const BiList_##Type *other) {    \
        BiList_##Type list = { 0, 0, other->size };    \
        if(other->first) {  \
            BiList_##Type##_Node *temp = other->first;  \
            list.first = list.last = (BiList_##Type##_Node*) calloc(1, sizeof(BiList_##Type##_Node));   \
            list.first->item = temp->item;  \
            temp = temp->next;  \
            while(temp) {   \
                list.last->next = (BiList_##Type##_Node*) calloc(1, sizeof(BiList_##Type##_Node));  \
                list.last->next->prev = list.last;  \
                list.last = list.last->next;    \
                list.last->item = temp->item;   \
                temp = temp->next;  \
            }   \
        }   \
        return list;    \
    }   \
    \
    void bilist_##Type##_push_front(BiList_##Type *self, const Type item) {    \
        if(self->first) {   \
            self->first->prev = (BiList_##Type##_Node*) calloc(1, sizeof(BiList_##Type##_Node));    \
            self->first->prev->next = self->first;  \
            self->first = self->first->prev;    \
        } else {    \
            self->first = self->last = (BiList_##Type##_Node*) calloc(1, sizeof(BiList_##Type##_Node)); \
        }   \
        self->first->item = item;   \
        ++self->size;   \
    }   \
    \
    void bilist_##Type##_push_back(BiList_##Type *self, const Type item) { \
        if(self->last) {    \
            self->last->next = (BiList_##Type##_Node*) calloc(1, sizeof(BiList_##Type##_Node)); \
            self->last->next->prev = self->last;    \
            self->last = self->last->next;  \
        } else {    \
            self->first = self->last = (BiList_##Type##_Node*) calloc(1, sizeof(BiList_##Type##_Node)); \
        }   \
        self->last->item = item;    \
        ++self->size;   \
    }   \
    \
    uint8_t bilist_##Type##_pop_front(BiList_##Type *self, Type *out) { \
        if(!self->first) return 0;  \
        *out = self->first->item;   \
        if(self->first->next) { \
            self->first = self->first->next;    \
            free(self->first->prev);    \
            self->first->prev = 0;  \
        } else {    \
            free(self->first);  \
            self->first = self->last = 0;   \
        }   \
        --self->size;   \
        return 1;   \
    }   \
    \
    uint8_t bilist_##Type##_pop_back(BiList_##Type *self, Type *out) {  \
        if(!self->last) return 0;   \
        *out = self->last->item;    \
        if(self->last->prev) {  \
            self->last = self->last->prev;  \
            free(self->last->next); \
            self->last->next = 0;   \
        } else {    \
            free(self->last);   \
            self->first = self->last = 0;   \
        }   \
        --self->size;   \
        return 1;   \
    }   \
    \
    static inline void bilist_##Type##_last(const BiList_##Type *self, Type *out) { \
        if(self->first) *out = self->first->item;   \
    }   \
    \
    static inline void bilist_##Type##_first(const BiList_##Type *self, Type *out) {    \
        if(self->last) *out = self->last->item; \
    }   \
    \
    Type* bilist_##Type##_to_array(const BiList_##Type *self) { \
        if(!self->first) return 0;  \
        Type *arr = (Type*) malloc(self->size*sizeof(Type));    \
        Type *ptr = arr;    \
        for(BiList_##Type##_Node *temp = self->first; temp; temp = temp->next) {    \
            *ptr++ = temp->item;    \
        }   \
        return arr; \
    }   \
    \
    Type* bilist_##Type##_to_array_reversed(const BiList_##Type *self) {    \
        if(!self->last) return 0;   \
        Type *arr = (Type*) malloc(self->size*sizeof(Type));    \
        Type *ptr = arr;    \
        for(BiList_##Type##_Node *temp = self->last; temp; temp = temp->prev) { \
            *ptr++ = temp->item;    \
        }   \
        return arr; \
    }   \
    \
    uint32_t bilist_##Type##_hash(const BiList_##Type *self) {  \
        uint32_t hash = 0;  \
        for(BiList_##Type##_Node *temp = self->first; temp; temp = temp->next) {    \
            hash ^= murmur(&temp->item, sizeof(Type));   \
        }   \
        return hash;    \
    }   \
    \
    void bilist_##Type##_free(BiList_##Type *self) {    \
        for(BiList_##Type##_Node *temp = self->first; temp; temp = self->first) {   \
            self->first = temp->next;   \
            free(temp); \
        }   \
        self->first = self->last = 0;   \
        self->size = 0; \
    }   \

#define bilist_foreach(Type, list, lambda) {    \
    for(BiList_##Type##_Node *temp = (list).first; temp; temp = temp->next) {   \
        lambda(temp->item); \
    }   \
}   \

#define bilist_foreach_reverse(Type, list, lambda) {    \
    for(BiList_##Type##_Node *temp = (list).last; temp; temp = temp->prev) {    \
        lambda(temp->item); \
    }   \
}   \

#endif
