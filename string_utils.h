#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <stddef.h>
#include <stdint.h>

size_t string_length(const char *_str) {
    union {
        const char *c;
        const uint64_t *l;
    } ptr;
    ptr.c = _str;
    while(*ptr.c && ((uintptr_t)ptr.c & 7)) ++ptr.c;
    while(!((*ptr.l - 0x0101010101010101UL) & ~*ptr.l & 0x8080808080808080UL)) ++ptr.l;
    if(!ptr.c[0]) return ptr.c-_str;
    if(!ptr.c[1]) return ptr.c-_str+1;
    if(!ptr.c[2]) return ptr.c-_str+2;
    if(!ptr.c[3]) return ptr.c-_str+3;
    if(!ptr.c[4]) return ptr.c-_str+4;
    if(!ptr.c[5]) return ptr.c-_str+5;
    if(!ptr.c[6]) return ptr.c-_str+6;
    if(!ptr.c[7]) return ptr.c-_str+7;
    return 0;
}

static inline char string_compare(const char *a, const char *b) {
    while(*a && *b && *a == *b) { ++a; ++b; }
    return *a-*b;
}

char* string_contains(const char *text, const char *pattern) {
    #define haszero64(x) (((x) - 0x0101010101010101UL) & ~(x) & 0x8080808080808080UL)
    #define hasbyte64(x, b) haszero64((x) ^ (b))
    
    union {
        char *ret;
        const uint8_t *u8;
        const uint64_t *u64;
    } text_ptr;
    text_ptr.u8 = (const uint8_t*)text;
    const uint8_t *pattern_u8 = (const uint8_t*)pattern;
    const uint8_t first = *pattern_u8;
    const uint64_t byte_64 = ~0UL/255*first;
    uint32_t i;
    while(*text_ptr.u8) {
        while(*text_ptr.u8 && *text_ptr.u8 != first && (uintptr_t)text_ptr.u8 & 7) ++text_ptr.u8;
        while(!haszero64(*text_ptr.u64) && !hasbyte64(*text_ptr.u64, byte_64)) ++text_ptr.u64;
        while(*text_ptr.u8 && *text_ptr.u8 != first) ++text_ptr.u8;
        i=1;
        while(text_ptr.u8[i] && pattern_u8[i] && text_ptr.u8[i] == pattern_u8[i]) ++i;
        i -= pattern_u8[i] == 0;
        if(pattern_u8[i] == text_ptr.u8[i]) return text_ptr.ret;
        ++text_ptr.u8;
    }
    return 0;
}

#endif
