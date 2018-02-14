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

String string_with_capacity(const uint64_t min) {
    if(min < 16) return (String) { 0 };
    uint64_t cap = 32;
    while(cap <= min) cap <<= 1;
    String str;
    str.long_str = (char*) calloc(cap, 1);
    str.cap = cap;
    str.length = 0;
    str.flag = 1;
    return str;
}

static inline char* string_chars(String *self) {
    return self->flag ? self->long_str : self->short_str;
}

static inline const char* string_const_chars(const String *self) {
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

void string_clear(String *self) {
    if(self->flag) {
        clear(self->long_str, self->length);
    } else {
        *self = (String) { 0 };
    }
    self->length = 0;
}

void string_assign_chars(String *self, const char *chars) {
    const uint64_t length = string_length(chars);
    if(self->flag) {
        if(length >= self->cap) {
            free(self->long_str);
            while(self->cap <= length) self->length <<= 1;
            self->long_str = (char*) calloc(self->cap, 1);
        } else {
            clear(self->long_str, self->length);
        }
        copy(self->long_str, chars, length);
    } else {
        if(length < 16) {
            copy(self->short_str, chars, length);
        } else {
            self->cap = 32;
            while(self->cap <= length) self->cap <<= 1;
            self->long_str = (char*) calloc(self->cap, 1);
            copy(self->long_str, chars, length);
        }
    }
    self->length = length;
}

void string_assign_string(String *self, const String *other) {
    switch(self->flag | (other->flag << 1)) {
        case 3: // Both long
            if(other->length >= self->cap) {
                free(self->long_str);
                while(self->cap <= other->length) self->cap <<= 1;
                self->long_str = (char*) calloc(self->cap, 1);
            } else {
                clear(self->long_str, self->length);
            }
            copy(self->long_str, other->long_str, other->length);
            break;
        case 2: // Only other long
            self->cap = 32;
            self->flag = 1;
            while(self->cap <= other->length) self->cap <<= 1;
            self->long_str = (char*) calloc(self->cap, 1);
            copy(self->long_str, other->long_str, other->length);
            break;
        case 1: // Only self long
            clear(self->long_str, self->length);
            copy(self->long_str, other->short_str, other->length);
            break;
        case 0: // Neither long
            *self = (String) { 0 };
            copy(self->short_str, other->short_str, other->length);
            break;
    }
    self->length = other->length;
}

void string_getline(String *self) {
    char buf[8192];
    int len = read(STDIN, buf, 8191) - 1;
    if(len > 0) {
        if(self->flag) {
            if(len >= self->cap) {
                while(self->cap <= len) self->cap <<= 1;
                free(self->long_str);
                self->long_str = (char*) calloc(self->cap, 1);
            } else {
                clear(self->long_str, self->length);
            }
            copy(self->long_str, buf, len);
        } else {
            if(len < 16) {
                self->cap = 32;
                self->flag = 1;
                while(self->cap <= len) self->cap <<= 1;
                self->long_str = (char*) calloc(self->cap, 1);
                copy(self->long_str, buf, len);
            } else {
                *self = (String) { 0 };
                copy(self->short_str, buf, len);
            }
        }
        self->length = len;
    }
}

String* string_split_chars(const String *self, const char *pattern, uint32_t *num_splits) {
    const uint64_t len_p = string_length(pattern);
    const char *str = self->flag ? self->long_str : self->short_str;
    const char *ptr = str;
    uint32_t num = 0;
    while((ptr = string_contains(ptr, pattern))) {
        ++num;
        ptr += len_p;
    }
    ++num;
    *num_splits = num;
    if(num == 0) return 0;
    ptr = str;
    String *splits = (String*) calloc(num, sizeof(String));
    uint32_t i=0;
    while((ptr = string_contains(str, pattern))) {
        splits[i].length = ptr-str;
        if(splits[i].length < 16) {
            copy(splits[i].short_str, str, splits[i].length);
        } else {
            splits[i].flag = 1;
            splits[i].cap = 32;
            while(splits[i].cap <= splits[i].length) splits[i].cap <<= 1;
            splits[i].long_str = (char*) calloc(splits[i].cap, 1);
            copy(splits[i].long_str, str, splits[i].length);
        }
        str = ptr+len_p;
        ++i;
    }
    splits[i].length = ((self->flag ? self->long_str : self->short_str)+self->length)-str;
    if(splits[i].length < 16) {
        copy(splits[i].short_str, str, splits[i].length);
    } else {
        splits[i].flag = 1;
        splits[i].cap = 32;
        while(splits[i].cap <= splits[i].length) splits[i].cap <<= 1;
        splits[i].long_str = (char*) calloc(splits[i].cap, 1);
        copy(splits[i].long_str, str, splits[i].length);
    }
    return splits;
}

void string_to_lower(String *self) {
#define is_upper(c) (c >= 'A' && c <= 'Z')
    char *str = self->flag ? self->long_str : self->short_str;
    for(uint64_t i=0; i<self->length; ++i) {
        if(is_upper(str[i])) str[i] += 'a'-'A';
    }
}

void string_to_upper(String *self) {
#define is_lower(c) (c >= 'a' && c <= 'z')
    char *str = self->flag ? self->long_str : self->short_str;
    for(uint64_t i=0; i< self->length; ++i) {
        if(is_lower(str[i])) str[i] -= 'a'-'A';
    }
}

void string_remove_whitespace(String *self) {
    char *str = self->flag ? self->long_str : self->short_str;
    uint64_t i=0, j=0;
    while(j < self->length) {
        while(str[j] <= 32) ++j;
        str[i++] = str[j++];
    }
    self->length = i;
    while(i < j) str[i++] = 0;
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

static inline char* string_contains_chars(const String *self, const char *chars) {
    return string_contains(self->flag ? self->long_str : self->short_str, chars);
}

static inline char* string_contains_string(const String *self, const String *other) {
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
