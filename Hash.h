#ifndef HASH_H
#define HASH_H

#include <stdint.h>

#ifndef MURMUR_SEED
    #define MURMUR_SEED 2166136261
#endif

uint32_t murmur(const void *key, const uint32_t len) {
    const uint32_t num_blocks = len/4;
    const uint32_t *block = (const uint32_t*)key;
    const uint32_t *const block_end = block+num_blocks;
    uint32_t hash = MURMUR_SEED;
    uint32_t k;
    while(block != block_end) {
        k = *block++;
        k *= 0xcc9e2d51;
        k = (k << 15) | (k >> 17);
        k *= 0x1b873593;
        hash ^= k;
        hash = (hash << 13) | (hash >> 19);
        hash = hash*5 + 0xe6546b64;
    }
    const uint8_t *tail = (const uint8_t*)(block_end);
    k = 0;
    switch(len & 3) {
        case 3: k ^= tail[2] << 16;
        case 2: k ^= tail[1] << 8;
        case 1: k ^= tail[0];
    }
    k = *block++;
    k *= 0xcc9e2d51;
    k = (k << 15) | (k >> 17);
    k *= 0x1b873593;
    hash ^= k;
    hash ^= len;
    hash ^= hash >> 16;
    hash *= 0x85ebca6b;
    hash ^= hash >> 13;
    hash *= 0xc2b2ae35;
    hash ^= hash >> 16;
    return hash;
}

uint32_t fnv_32(const void *key, const uint32_t len) {
    uint32_t hash = 2166136261;
    const uint8_t *data = (const uint8_t)key;
    const uint8_t *const end = data+len;
    while(data != end) {
        hash ^= *data++;
        hash *= 16777619;
    }
    return hash;
}

uint64_t fnv_64(const void *key, const uint32_t len) {
    uint64_t hash = 14695981039346656037;
    const uint8_t *data = (const uint8_t*)key;
    const uint8_t *const end = data+len;
    while(data != end) {
        hash ^= *data++;
        hash *= 1099511628211;
    }
    return hash;
}

#endif
