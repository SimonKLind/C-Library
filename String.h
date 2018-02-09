#ifndef STRING_H
#define STRING_H

#include <stdint.h>
#include <std/memory.h>
#include <std/syscalls.h>
#include <std/string_utils.h>
#include <std/Hash.h>

typedef struct String {
    union {
        struct {
            char *long_str;
            uint64_t cap;
        };
        char short_str[16];
    };
    uint64_t length: 63;
    uint8_t flag: 1;
} String;

static inline String string_new() {
    return (String) { 0 };
}

static inline char* string_chars(String *self) {
    return self->flag ? self->long_str : self->short_str;
}

static inline void string_print(const String *self) {
    write(STDOUT, self->flag ? self->long_str : self->short_str, self->length);
}

static inline uint32_t string_hash(const String *self) {
    return murmur(self->flag ? self->long_str : self->short_str, self->length);
}

static inline void string_free(String *self) {
    if(self->flag) free(self->long_str);
    *self = (String) { 0 };
}

String string_copy(const String *other) {
    if(!other->flag) return *other;
    String str;
    str.length = other->length;
    str.flag = other->flag;
    str.cap = other->cap;
    str.long_str = (char*) calloc(str.cap, 1);
    copy(str.long_str, other->long_str, str.length);
    return str;
}

String string_from_chars(const char *chars) {
    String str = { 0 };
    str.length = string_length(chars);
    if(str.length < 16) {
        copy(str.short_str, chars, str.length);
    } else {
        str.cap = 32;
        str.flag = 1;
        while(str.cap <= str.length) str.cap <<= 1;
        str.long_str = (char*) calloc(str.cap, 1);
        copy(str.long_str, chars, str.length);
    }
    return str;
}

String string_from_file(const char *filename) {
    int fd = open(filename, O_RDONLY);
    String str = { 0 };
    if(fd > 0) {
        str.length = lseek(fd, 0, END);
        lseek(fd, 0, BEG);
        if(str.length < 16) {
            read(fd, str.short_str, str.length);
        } else {
            str.cap = 32;
            str.flag = 1;
            while(str.cap <= str.length) str.cap <<= 1;
            str.long_str = (char*) calloc(str.cap, 1);
            read(fd, str.long_str, str.length);
        }
        close(fd);
    }
    return str;
}

void string_write_to_file(const String *self, const char *filename) {
    int fd = open(filename, O_WRONLY | O_TRUNC | O_CREAT, 0777);
    if(fd > 0) {
        write(fd, self->flag ? self->long_str : self->short_str, self->length);
        close(fd);
    }
}

void string_reserve_short(String *self, const uint64_t size) {
    uint64_t cap = 32;
    while(cap <= size) cap <<= 1;
    char *new_str = (char*) calloc(cap, 1);
    copy(new_str, self->short_str, self->length);
    self->long_str = new_str;
    self->cap = cap;
    self->flag = 1;
}

void string_reserve_long(String *self, const uint64_t size) {
    char *old = self->long_str;
    while(self->cap <= size) self->cap <<= 1;
    self->long_str = (char*) calloc(self->cap, 1);
    copy(self->long_str, old, self->length);
    free(old);
}

void string_append_chars(String *self, const char *chars) {
    const uint64_t length = string_length(chars);
    if(self->flag) {
        if(self->length + length >= self->cap) string_reserve_long(self, self->length + length);
        copy(self->long_str + self->length, chars, length);
    } else {
        if(self->length + length >= 16) {
            string_reserve_short(self, self->length + length);
            copy(self->long_str + self->length, chars, length);
        } else {
            copy(self->short_str + self->length, chars, length);
        }
    }
    self->length += length;
}

void string_append_string(String *self, const String *other) {
    switch(self->flag | (other->flag << 1)) {
        case 3: // Both long
            if(self->length + other->length >= self->cap) string_reserve_long(self, self->length + other->length);
            copy(self->long_str + self->length, other->long_str, other->length);
            break;
        case 2: // Only other long
            string_reserve_short(self, self->length + other->length);
            copy(self->long_str + self->length, other->long_str, other->length);
            break;
        case 1: // Only self long
            if(self->length + other->length >= self->cap) string_reserve_long(self, self->length + other->length);
            copy(self->long_str + self->length, other->short_str, other->length);
            break;
        case 0: // Neither long
            if(self->length + other->length >= 16) {
                string_reserve_short(self, self->length + other->length);
                copy(self->long_str + self->length, other->short_str, other->length);
            } else {
                copy(self->short_str + self->length, other->short_str, other->length);
            }
            break;
    }
    self->length += other->length;
}

void string_append(String *self, const char c) {
    if(self->flag) {
        if(self->length+1 >= self->cap) string_reserve_long(self, self->cap);
        self->long_str[self->length++] = c;
    } else {
        if(self->length+1 >= 16) {
            string_reserve_short(self, 16);
            self->long_str[self->length++] = c;
        } else {
            self->short_str[self->length++] = c;
        }
    }
}

static inline uint8_t string_contains_chars(const String *self, const char *chars) {
    return string_contains(self->flag ? self->long_str : self->short_str, chars);
}

static inline uint8_t string_contains_string(const String *self, const String *other) {
    return string_contains(self->flag ? self->long_str : self->short_str, other->flag ? other->long_str : other->short_str);
}

static inline char string_compare_chars(const String *self, const char *chars) {
    return string_compare(self->flag ? self->long_str : self->short_str, chars);
}

static inline int string_compare_string(const String *self, const String *other) {
    if(self->length != other->length) return self->length - other->length;
    return string_compare(self->flag ? self->long_str : self->short_str, other->flag ? other->long_str : other->short_str);
}

#endif
