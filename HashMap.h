#ifndef HASHMAP_H
#define HASHMAP_H

#include <std/memory.h>
#include <stdint.h>
#include <std/Hash.h>

#ifndef RESIZE_THRESHOLD
    #define RESIZE_THRESHOLD 0.75
#endif

#define init_hashmap(Key, Val, key_equals, key_hash)    \
    typedef struct HashMap_##Key##_##Val##_Node {   \
        Key key;    \
        Val val;    \
        struct HashMap_##Key##_##Val##_Node *next;  \
    } HashMap_##Key##_##Val##_Node; \
    \
    typedef struct HashMap_##Key##_##Val {  \
        HashMap_##Key##_##Val##_Node **buckets; \
        uint32_t num_buckets;   \
        uint32_t size;  \
    } HashMap_##Key##_##Val;    \
    \
    static inline HashMap_##Key##_##Val hashmap_##Key##_##Val##_new() { \
        return (HashMap_##Key##_##Val) { (HashMap_##Key##_##Val##_Node**) calloc(16, sizeof(HashMap_##Key##_##Val##_Node*)), 16, 0 };   \
    }   \
    \
    HashMap_##Key##_##Val hashmap_##Key##_##Val##_copy(const HashMap_##Key##_##Val *other) {    \
        HashMap_##Key##_##Val map = (HashMap_##Key##_##Val) {   \
            (HashMap_##Key##_##Val##_Node**) calloc(other->num_buckets, sizeof(HashMap_##Key##_##Val##_Node*)), \
            other->num_buckets, \
            other->size \
        };  \
        HashMap_##Key##_##Val##_Node *new_temp; \
        for(uint32_t i=0; i<other->num_buckets; ++i) {  \
            for(HashMap_##Key##_##Val##_Node *temp = other->buckets[i]; temp; temp = temp->next) {  \
                new_temp = (HashMap_##Key##_##Val##_Node*) malloc(sizeof(HashMap_##Key##_##Val##_Node));    \
                *new_temp = (HashMap_##Key##_##Val##_Node) { temp->key, temp->val, map.buckets[i] };    \
                map.buckets[i] = new_temp;  \
            }   \
        }   \
        return map; \
    }   \
    \
    void hashmap_##Key##_##Val##_expand(HashMap_##Key##_##Val *self) {  \
        HashMap_##Key##_##Val##_Node **old_buckets = self->buckets; \
        const uint32_t old_num_buckets = self->num_buckets;   \
        self->num_buckets <<= 1;    \
        self->buckets = (HashMap_##Key##_##Val##_Node**) calloc(self->num_buckets, sizeof(HashMap_##Key##_##Val##_Node*));  \
        HashMap_##Key##_##Val##_Node *temp; \
        for(uint32_t i=0, index; i<old_num_buckets; ++i) {  \
            while(old_buckets[i]) { \
                index = key_hash(old_buckets[i]->key)&(self->num_buckets-1);    \
                temp = old_buckets[i];  \
                old_buckets[i] = old_buckets[i]->next;  \
                temp->next = self->buckets[index];  \
                self->buckets[index] = temp;    \
            }   \
        }   \
        free(old_buckets);  \
    }   \
    \
    void hashmap_##Key##_##Val##_put(HashMap_##Key##_##Val *self, const Key key, const Val val) {   \
        const uint32_t index = key_hash(key)&(self->num_buckets-1); \
        HashMap_##Key##_##Val##_Node *temp; \
        for(temp = self->buckets[index]; temp; temp = temp->next) { \
            if(key_equals(temp->key, key)) {    \
                temp->val = val;    \
                return; \
            }   \
        }   \
        temp = self->buckets[index];    \
        self->buckets[index] = (HashMap_##Key##_##Val##_Node*) malloc(sizeof(HashMap_##Key##_##Val##_Node));    \
        *self->buckets[index] = (HashMap_##Key##_##Val##_Node) { key, val, temp };  \
        if(++self->size >= RESIZE_THRESHOLD*self->num_buckets) hashmap_##Key##_##Val##_expand(self);    \
    }   \
    \
    uint8_t hashmap_##Key##_##Val##_get(const HashMap_##Key##_##Val *self, const Key key, Val *val) {   \
        const uint32_t index = key_hash(key)&(self->num_buckets-1); \
        for(HashMap_##Key##_##Val##_Node *temp = self->buckets[index]; temp; temp = temp->next) {   \
            if(key_equals(temp->key, key)) {    \
                *val = temp->val;   \
                return 1;   \
            }   \
        }   \
        return 0;   \
    }   \
    \
    uint8_t hashmap_##Key##_##Val##_contains(const HashMap_##Key##_##Val *self, const Key key) {    \
        const uint32_t index = key_hash(key)&(self->num_buckets-1); \
        for(HashMap_##Key##_##Val##_Node *temp = self->buckets[index]; temp; temp = temp->next) {   \
            if(key_equals(temp->key, key)) return 1;    \
        }   \
        return 0;   \
    }   \
    \
    uint8_t hashmap_##Key##_##Val##_remove(HashMap_##Key##_##Val *self, const Key key, Val *val) {  \
        const uint32_t index = key_hash(key)&(self->num_buckets-1); \
        if(!self->buckets[index]) return 0; \
        if(key_equals(self->buckets[index]->key, key)) {    \
            if(val) *val = self->buckets[index]->val;   \
            HashMap_##Key##_##Val##_Node *temp = self->buckets[index];    \
            self->buckets[index] = self->buckets[index]->next;  \
            free(temp); \
            --self->size;   \
            return 1;   \
        }   \
        for(HashMap_##Key##_##Val##_Node *temp = self->buckets[index], *next = self->buckets[index]->next; next; temp = next, next = next->next) {   \
            if(key_equals(next->key, key)) {    \
                if(val) *val = next->val;   \
                temp->next = next->next;    \
                free(next);   \
                --self->size;   \
                return 1;   \
            }   \
        }   \
        return 0;   \
    }   \
    \
    void hashmap_##Key##_##Val##_to_array(const HashMap_##Key##_##Val *self, Key *keys, Val *vals) {    \
        if(keys && vals) { /* Both seem valid, copy into both */    \
            for(uint32_t i=0; i<self->num_buckets; ++i) {   \
                for(const HashMap_##Key##_##Val##_Node *temp = self->buckets[i]; temp; temp = temp->next) { \
                    *keys++ = temp->key;    \
                    *vals++ = temp->val;    \
                }   \
            }   \
        } else if(keys) { /* Only keys seem valid, copy only into keys */   \
            for(uint32_t i=0; i<self->num_buckets; ++i) {   \
                for(const HashMap_##Key##_##Val##_Node *temp = self->buckets[i]; temp; temp = temp->next) { \
                    *keys++ = temp->key;    \
                }   \
            }   \
        } else if(vals) { /* Only vals seem valid, copy only into vals */   \
            for(uint32_t i=0; i<self->num_buckets; ++i) {   \
                for(const HashMap_##Key##_##Val##_Node *temp = self->buckets[i]; temp; temp = temp->next) { \
                    *vals++ = temp->val;    \
                }   \
            }   \
        }   \
    }   \
    \
    void hashmap_##Key##_##Val##_free(HashMap_##Key##_##Val *self) {    \
        for(uint32_t i=0; i<self->num_buckets; ++i) {   \
            for(HashMap_##Key##_##Val##_Node *temp = self->buckets[i]; temp; temp = self->buckets[i]) { \
                self->buckets[i] = self->buckets[i]->next;  \
                free(temp); \
            }   \
        }   \
        free(self->buckets);    \
        *self = (HashMap_##Key##_##Val) { 0 };  \
    }   \

#define hashmap_foreach(Key, Val, map, lambda) {    \
    for(uint32_t _i = 0; _i<(map).num_buckets; ++_i) {  \
        for(HashMap_##Key##_##Val##_Node *_temp = (map).buckets[_i]; _temp; _temp = _temp->next) {   \
            lambda(_temp->key, _temp->val); \
        }   \
    }   \
}   \

#endif
