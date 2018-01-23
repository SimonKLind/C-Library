#ifndef STACK_H
#define STACK_H

#include <memory.h>
#include <stdint.h>
#include <std/Hash.h>

#define init_stack(Type)    \
    typedef struct Stack_##Type##_Node {    \
        Type item;  \
        struct Stack_##Type##_Node *next;  \
    } Stack_##Type##_Node;  \
    \
    typedef struct Stack_##Type {   \
        Stack_##Type##_Node *first; \
        uint32_t size;  \
    } Stack_##Type; \
    \
    static inline Stack_##Type stack_##Type##_new() {   \
        return (Stack_##Type) { 0, 0 }; \
    }   \
    \
    Stack_##Type stack_##Type##_copy(Stack_##Type *other) { \
        Stack_##Type stack = { 0, other->size };    \
        if(other->first) {  \
            Stack_##Type##_Node *temp = other->first;   \
            stack.first = (Stack_##Type##_Node*) malloc(sizeof(Stack_##Type##_Node));   \
            stack.first->item = temp->item;  \
            Stack_##Type##_Node *node = stack.first;    \
            temp = temp->next;  \
            while(temp) {   \
                node->next = (Stack_##Type##_Node*) malloc(sizeof(Stack_##Type##_Node));    \
                *node->next = (Stack_##Type##_Node) { temp->item, 0 };  \
                node = node->next;  \
                temp = temp->next;  \
            }   \
        }   \
        return stack;   \
    }   \
    \
    static inline void stack_##Type##_push(Stack_##Type *self, Type item) { \
        Stack_##Type##_Node *temp = (Stack_##Type##_Node*) malloc(sizeof(Stack_##Type##_Node));   \
        *temp = (Stack_##Type##_Node) { item, self->first };    \
        self->first = temp; \
        ++self->size;   \
    }   \
    \
    uint8_t stack_##Type##_pop(Stack_##Type *self, Type *out) { \
        if(!self->first) return 0;  \
        Stack_##Type##_Node *temp = self->first;    \
        self->first = temp->next;   \
        *out = temp->item;   \
        free(temp); \
        --self->size;   \
        return 1;   \
    }   \
    \
    static inline void stack_##Type##_peek(Stack_##Type *self, Type *out) { \
        if(self->first) *out = self->first->item;    \
    }   \
    \
    Type* stack_##Type##_to_array(const Stack_##Type *self) {   \
        if(!self->first) return 0;  \
        Type *arr = (Type*) malloc(self->size*sizeof(Type)); \
        Type *ptr = arr;    \
        for(Stack_##Type##_Node *temp = self->first; temp; temp = temp->next) {   \
            *ptr++ = temp->item;    \
        }   \
        return arr; \
    }   \
    \
    uint32_t stack_##Type##_hash(const Stack_##Type *self) {    \
        uint32_t hash = 0;  \
        for(Stack_##Type##_Node *temp = self->first; temp; temp = temp->next) {   \
            hash ^= murmur(&temp->item, sizeof(Type));   \
        }   \
        return hash;    \
    }   \
    \
    void stack_##Type##_free(Stack_##Type *self) {  \
        for(Stack_##Type##_Node *temp = self->first; temp; temp = self->first) {   \
            self->first = temp->next;   \
            free(temp); \
        }   \
        self->first = 0; \
        self->size = 0; \
    }   \


#define stack_foreach(Type, stack, lambda) {    \
    for(Stack_##Type##_Node *_temp = (stack).first; _temp; _temp = _temp->next) {  \
        lambda(_temp->item);    \
    }   \
}   \
    
#endif
