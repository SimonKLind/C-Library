#ifndef HASHMAP_H
#define HASHMAP_H

#include <std/memory.h>
#include <stdint.h>
#include <std/Hash.h>

#ifndef LOAD_FACTOR
    #define LOAD_FACTOR 0.9
#endif

#define init_hashmap(Key, Val, key_equals, key_hash)    \
    typedef struct HashMap_##Key##_##Val##_Node {   \
        Key key;    \
        Val val;    \
        uint32_t hash;  \
        uint16_t active;    \
        uint16_t alive; \
    } HashMap_##Key##_##Val##_Node; \
    \
    typedef struct HashMap_##Key##_##Val {  \
        HashMap_##Key##_##Val##_Node *nodes;    \
        uint32_t size;  \
        uint32_t cap;   \
    } HashMap_##Key##_##Val;    \
    \
    static inline HashMap_##Key##_##Val hashmap_##Key##_##Val##_new() { \
        return (HashMap_##Key##_##Val) { (HashMap_##Key##_##Val##_Node*) calloc(16, sizeof(HashMap_##Key##_##Val##_Node)), 0, 16 }; \
    }   \
    \
    static inline HashMap_##Key##_##Val hashmap_##Key##_##Val##_copy(const HashMap_##Key##_##Val *other) {  \
        HashMap_##Key##_##Val map = { (HashMap_##Key##_##Val##_Node*) calloc(other->cap, sizeof(HashMap_##Key##_##Val##_Node)), other->size, other->cap };  \
        copy(map.nodes, other->nodes, other->cap*sizeof(HashMap_##Key##_##Val##_Node)); \
        return map; \
    }   \
    \
    void hashmap_##Key##_##Val##_expand(HashMap_##Key##_##Val *self) {  \
        HashMap_##Key##_##Val##_Node *old = self->nodes;    \
        const uint32_t old_cap = self->cap; \
        self->cap <<= 1;    \
        self->nodes = (HashMap_##Key##_##Val##_Node*) calloc(self->cap, sizeof(HashMap_##Key##_##Val##_Node));  \
        const uint32_t mask = self->cap-1;  \
        HashMap_##Key##_##Val##_Node temp;  \
        for(uint32_t i=0, index; i<old_cap; ++i) { \
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
    void hashmap_##Key##_##Val##_put(HashMap_##Key##_##Val *self, const Key key, const Val val) {   \
        const uint32_t mask = self->cap-1;  \
        const uint32_t hash = key_hash(key);    \
        uint32_t index = hash&mask; \
        HashMap_##Key##_##Val##_Node current = { key, val, hash, 1, 1 }, temp; \
        for(uint32_t dist = 0; self->nodes[index].active; ++dist) { \
            if(self->nodes[index].hash == hash && key_equals(self->nodes[index].key, key)) {    \
                self->nodes[index].val = val;   \
                return; \
            } else if(index-(self->nodes[index].hash&mask)+self->cap*(index-(self->nodes[index].hash&mask)>=self->cap) < dist) {    \
                temp = self->nodes[index];  \
                self->nodes[index] = current;   \
                current = temp; \
            }   \
            index = (index+1)&mask; \
        }   \
        self->nodes[index] = current;   \
        if(++self->size >= LOAD_FACTOR*self->cap) hashmap_##Key##_##Val##_expand(self); \
    }   \
    \
    uint8_t hashmap_##Key##_##Val##_get(const HashMap_##Key##_##Val *self, const Key key, Val *val) {   \
        const uint32_t hash = key_hash(key);    \
        const uint32_t mask = self->cap-1;  \
        for(uint32_t index = hash&mask, dist = 0;   \
            self->nodes[index].alive && index-(self->nodes[index].hash&mask)+self->cap*(index-(self->nodes[index].hash&mask)>=self->cap) >= dist;    \
            index = (index+1)&mask, ++dist) {   \
            if(self->nodes[index].active && self->nodes[index].hash == hash && key_equals(self->nodes[index].key, key)) {    \
                *val = self->nodes[index].val;  \
                return 1;   \
            }   \
        }   \
        return 0;   \
    }   \
    \
    uint8_t hashmap_##Key##_##Val##_contains(const HashMap_##Key##_##Val *self, const Key key) {    \
        const uint32_t hash = key_hash(key);    \
        const uint32_t mask = self->cap-1;  \
        for(uint32_t index = hash&mask, dist = 0;   \
            self->nodes[index].alive && index-(self->nodes[index].hash&mask)+self->cap*(index-(self->nodes[index].hash&mask)>=self->cap) >= dist;    \
            index = (index+1)&mask, ++dist) {   \
            if(self->nodes[index].active && self->nodes[index].hash == hash && key_equals(self->nodes[index].key, key)) return 1;    \
        }   \
        return 0;   \
    }   \
    \
    uint8_t hashmap_##Key##_##Val##_remove(HashMap_##Key##_##Val *self, const Key key, Val *val) {  \
        const uint32_t hash = key_hash(key);    \
        const uint32_t mask = self->cap-1;  \
        for(uint32_t index = hash&mask, dist = 0;   \
            self->nodes[index].alive && index-(self->nodes[index].hash&mask)+self->cap*(index-(self->nodes[index].hash&mask)>=self->cap) >= dist;    \
            index = (index+1)&mask, ++dist) {   \
            if(self->nodes[index].active && self->nodes[index].hash == hash && key_equals(self->nodes[index].key, key)) {    \
                if(val) *val = self->nodes[index].val;  \
                self->nodes[index].active = 0;  \
                --self->size;   \
                return 1;   \
            }   \
        }   \
        return 0;   \
    }   \
    \
    void hashmap_##Key##_##Val##_to_array(const HashMap_##Key##_##Val *self, Key *keys, Val *vals) {    \
        switch((keys != 0) | ((vals != 0) << 1)) {  \
            case 3: \
                for(uint32_t i=0; i<self->cap; ++i) {   \
                    if(!self->nodes[i].active) continue;    \
                    *keys++ = self->nodes[i].key;   \
                    *vals++ = self->nodes[i].val;   \
                }   \
                break;  \
            case 2: \
                for(uint32_t i=0; i<self->cap; ++i) {   \
                    if(!self->nodes[i].active) continue;    \
                    *vals++ = self->nodes[i].val;   \
                }   \
                break;  \
            case 1: \
                for(uint32_t i=0; i<self->cap; ++i) {   \
                    if(!self->nodes[i].active) continue;    \
                    *keys++ = self->nodes[i].key;  \
                }   \
        }   \
    }   \
    \
    static inline void hashmap_##Key##_##Val##_free(HashMap_##Key##_##Val *self) {    \
        free(self->nodes);  \
    }   \

#define hashmap_foreach(Key, Val, map, lambda) {    \
    for(uint32_t _i=0; _i<(map).cap; ++_i) {    \
        if(!(map).nodes[_i].active) continue;   \
        lambda((map).nodes[_i].key, (map).nodes[_i].val);   \
    }   \
}   \

#endif
