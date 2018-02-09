#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>
#include <stdint.h>

void copy(void *restrict _dest, const void *restrict _source, const size_t len) {
    union {
        uint8_t *u8;
        uint16_t *u16;
        uint32_t *u32;
        uint64_t *u64;
    } dest;
    dest.u8 = (uint8_t*) _dest;
    union {
        const uint8_t *u8;
        const uint16_t *u16;
        const uint32_t *u32;
        const uint64_t *u64;
    } source;
    source.u8 = (const uint8_t*)_source;
    void *end = _dest+len;
    if(len > 128) {
        while((uintptr_t)dest.u8 & 7) *dest.u8++ = *source.u8;
        void *word_end;
        switch((uintptr_t)source.u8 & 7) {
            case 0:
                word_end = (void*)((uintptr_t)end & ~7);
                while(dest.u64 != word_end) *dest.u64++ = *source.u64++;
                break;
            case 4:
                word_end = (void*)((uintptr_t)end & ~3);
                while(dest.u32 != word_end) *dest.u32++ = *source.u32++;
                break;
            case 2:
                word_end = (void*)((uintptr_t)end & ~1);
                while(dest.u16 != word_end) *dest.u16++ = *source.u16++;
                break;
        }
    }
    while(dest.u8 != end) *dest.u8++ = *source.u8++;
}

void* malloc(size_t size);
void* calloc(size_t num, size_t size);
void free(void *ptr);

#endif
