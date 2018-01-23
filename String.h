#ifndef STRING_H
#define STRING_H

#include <stddef.h>
#include <memory.h>
#include <syscalls.h>
#include <stdint.h>
#include <std/Hash.h>

size_t strlen(const char *str);
int strcmp(const char *lhs, const char *rhs);

#define BASE_LENGTH 16

typedef struct String {
    char base[BASE_LENGTH];
    char *str;
    uint32_t length;
    uint32_t cap;
} String;

static inline String string_new() {
    return (String) { {0}, 0, 0, BASE_LENGTH };
}

static inline char* string_chars(String *self) {
    return self->str ? self->str : self->base;
}

static inline void string_print(const String *self) {
    write(STDOUT, self->str ? self->str : self->base, self->length);
}

static inline int string_strcmp(const String *lhs, const String *rhs) {
    return strcmp(lhs->str ? lhs->str : lhs->base, rhs->str ? rhs->str : rhs->base);
}

static inline uint32_t string_hash(const String *self) {
    return murmur(self->str ? self->str : self->base, self->length);
}

static inline void string_free(String *self) {
    if(self->str) free(self->str);
}

String string_copy(const String *other) {
    String str = { {0}, 0, other->length, other->cap };
    if(other->str) {
        str.str = (char*) calloc(other->cap, 1);
        memcpy(str.str, other->str, other->length);
    } else {
        memcpy(str.base, other->base, other->length);
    }
    return str;
}

String string_from_chars(const char *chars) {
    const uint32_t length = strlen(chars);
    uint32_t cap = BASE_LENGTH;
    while(cap <= length) cap <<= 1;
    String str = { {0}, 0, length, cap };
    if(cap == BASE_LENGTH) {
        memcpy(str.base, chars, length);
    } else {
        str.str = (char*) calloc(cap, 1);
        memcpy(str.str, chars, length);
    }
    return str;
}

String string_from_file(const char *filename) {
    int fd = open(filename, O_RDONLY);
    String str = { {0}, 0, 0, BASE_LENGTH };
    if(fd > 0) {
        str.length = lseek(fd, 0, END);
        lseek(fd, 0, BEG);
        while(str.cap <= str.length) str.cap <<= 1;
        if(str.cap == BASE_LENGTH) {
            read(fd, str.base, str.length);
        } else {
            str.str = (char*) calloc(str.cap, 1);
            read(fd, str.str, str.length);
        }
        close(fd);
    } else {
        printf("Error opening %s, returning empty string!\n", filename);
    }
    return str;
}

void string_write_to_file(const String *self, const char *filename) {
    int fd = open(filename, O_WRONLY | O_TRUNC | O_CREAT, 0777);
    if(fd > 0) {
        write(fd, self->str ? self->str : self->base, self->length);
        close(fd);
    } else {
        printf("Error opening %s!", filename);
    }
}

void string_reserve(String *self, uint32_t size) {
    const char str = self->str != 0;
    char *old = str ? self->str : self->base;
    while(self->cap <= size) self->cap <<= 1;
    self->str = (char*) calloc(self->cap, 1);
    memcpy(self->str, old, self->length);
    if(str) free(old);
}

void string_append_chars(String *self, const char *chars) {
    const uint32_t length = strlen(chars);
    if(self->length + length >= self->cap) string_reserve(self, self->length+length);
    memcpy((self->str ? self->str : self->base) + self->length, chars, length);
    self->length += length;
}

void string_append_string(String *self, const String *other) {
    if(self->length + other->length >= self->cap) string_reserve(self, self->length+other->length);
    memcpy((self->str ? self->str : self->base) + self->length, other->str ? other->str : other->base, other->length);
    self->length += other->length;
}

void string_append(String *self, const char c) {
    (self->str ? self->str : self->base)[self->length++] = c;
    if(self->length >= self->cap) string_reserve(self, self->cap);
}

#endif
