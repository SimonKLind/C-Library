#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <std/memory.h>
#include <stdint.h>
#include <std/Hash.h>

#define init_list(Type) \
    typedef struct List_##Type##_Node { \
        Type item;  \
        struct List_##Type##_Node *next;    \
    } List_##Type##_Node;   \
    \
    typedef struct List_##Type {    \
        List_##Type##_Node *first;  \
        List_##Type##_Node *last;   \
        uint32_t size;  \
    } List_##Type;  \
    \
    static inline List_##Type list_##Type##_new() { \
        return (List_##Type) { 0, 0, 0 };   \
    }   \
    \
    List_##Type list_##Type##_copy(const List_##Type *other) {  \
        List_##Type list = { 0, 0, other->size };   \
        if(other->first) {  \
            List_##Type##_Node *temp = other->first;    \
            list.first = list.last = (List_##Type##_Node*) calloc(1, sizeof(List_##Type##_Node));   \
            list.first->item = temp->item;  \
            temp = temp->next;  \
            while(temp) {   \
                list.last->next = (List_##Type##_Node*) calloc(1, sizeof(List_##Type##_Node));  \
                list.last = list.last->next;    \
                list.last->item = temp->item;   \
                temp = temp->next;  \
            }   \
        }   \
        return list;    \
    }   \
    \
    void list_##Type##_push(List_##Type *self, const Type item) { \
        if(self->last) {    \
            self->last->next = (List_##Type##_Node*) calloc(1, sizeof(List_##Type##_Node)); \
            self->last = self->last->next;  \
        } else {    \
            self->last = self->first = (List_##Type##_Node*) calloc(1, sizeof(List_##Type##_Node)); \
        }   \
        self->last->item = item;    \
        ++self->size;   \
    }   \
    \
    uint8_t list_##Type##_pop(List_##Type *self, Type *out) {   \
        if(!self->first) return 0;  \
        List_##Type##_Node *temp = self->first; \
        if(temp->next) {    \
            self->first = temp->next;   \
        } else {    \
            self->last = self->first = 0;   \
        }   \
        *out = temp->item;  \
        free(temp); \
        --self->size; \
        return 1;   \
    }   \
    \
    static inline void list_##Type##_peek(const List_##Type *self, Type *out) { \
        if(self->first) *out = self->first->item;   \
    }   \
    \
    Type* list_##Type##_to_array(const List_##Type *self) { \
        if(!self->first) return 0;  \
        Type *arr = (Type*) malloc(self->size*sizeof(Type));    \
        Type *ptr = arr;    \
        for(List_##Type##_Node *temp = self->first; temp; temp = temp->next) {  \
            *ptr++ = temp->item;    \
        }   \
        return arr; \
    }   \
    \
    uint32_t list_##Type##_hash(const List_##Type *self) {  \
        uint32_t hash = 0;  \
        for(List_##Type##_Node *temp = self->first; temp; temp = temp->next) {  \
            hash ^= murmur(&temp->item, sizeof(Type));  \
        }   \
        return hash;    \
    }   \
    \
    void list_##Type##_free(List_##Type *self) {    \
        for(List_##Type##_Node *temp = self->first; temp; temp = self->first) { \
            self->first = temp->next;   \
            free(temp); \
        }   \
        self->first = self->last = 0;   \
        self->size = 0; \
    }   \

#define list_foreach(Type, list, lambda) {  \
    for(List_##Type##_Node *_temp = (list).first; _temp; _temp = _temp->next) { \
        lambda(_temp->item);    \
    }   \
}   \

#endif
