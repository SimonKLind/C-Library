#ifndef PRINT_H
#define PRINT_H

#include <syscalls.h>
#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>

#ifndef BUFFER_SIZE
    #define BUFFER_SIZE 8192
#endif

#define println(fmt, ...) print(fmt "\n", __VA_ARGS__)

static inline void reverse(unsigned char *start, unsigned char *end){
    while((ptrdiff_t)(end-start) > 0) {
        *start ^= *end;
        *end ^= *start;
        *start++ ^= *end--;
    } 
}

uint32_t parse_int(unsigned char *buf, long val) {
    uint32_t pos = 0;
    uint8_t sign = *(unsigned long*)&val >> 63;
    if(sign) {
        buf[pos++] = '-';
        val = -val;
    }
    do {
        buf[pos++] = (unsigned char)((val%10) + '0');
        val /= 10;
    } while(val);
    reverse(buf+sign, buf+pos-1);
    return pos;
}

static inline uint32_t parse_uint(unsigned char *buf, unsigned long val) {
    uint32_t pos = 0;
    do {
        buf[pos++] = (unsigned char)((val%10) + '0');
        val /= 10;
    } while(val);
    reverse(buf, buf+pos-1);
    return pos;
}

uint32_t parse_uint_hex(unsigned char *buf, unsigned long val) {
    static unsigned char hexdigits[16] = "0123456789ABCDEF";
    uint32_t pos = 0;
    do {
        buf[pos++] = hexdigits[val&15];
        val >>= 4;
    } while(val);
    reverse(buf, buf+pos-1);
    return pos;
}

static inline uint32_t uintlog10(uint32_t val) {
    return val >= 1000000000 ? 9 :
            val >= 100000000 ? 8 :
            val >= 10000000  ? 7 :
            val >= 1000000   ? 6 :
            val >= 100000    ? 5 :
            val >= 10000     ? 4 :
            val >= 1000      ? 3 :
            val >= 100       ? 2 :
            val >= 10        ? 1 : 0;
}

const double pow_10[10] = { 1.0, 0.1, 0.01, 0.001, 0.0001, 0.00001, 0.000001, 0.0000001, 0.00000001, 0.000000001 };

uint32_t parse_double(unsigned char *buf, double val) {
    uint32_t pos = 0;
    uint8_t sign = *(unsigned long*)&val >> 63;
    if(sign) {
        buf[pos++] = '-';
        val = -val;
    }
    int32_t exponent = 0;
    uint32_t u;
    while(val < 1.0) {
        val *= 1000000000.0;
        exponent -= 9;
    } 
    while(val >= 1.0e10) {
        val *= 0.000000001;
        exponent += 9;
    }
    u = val;
    uint32_t e = uintlog10(u);
    exponent += e;
    val *= pow_10[e];
    u = val;
    buf[pos++] = u%10 + '0';
    buf[pos++] = '.';
    val -= u;
    val *= 1000000000.0;
    u = val;
    pos += parse_uint(buf+pos, u);
    if(exponent) {
        buf[pos++] = 'e';
        if(*(uint32_t*)&exponent >> 31){
            buf[pos++] = '-';
            exponent = -exponent;
        }
        pos += parse_uint(buf+pos, exponent);
    }
    return pos;
}

#define haszero64(x) (((x) - 0x0101010101010101UL) & ~(x) & 0x8080808080808080UL)
#define hasmod64(x) haszero64(x ^ 0x2525252525252525UL)
#define align64(x) (((uintptr_t)x & 7) == 0)

#define haszero32(x) (((x) - 0x01010101U) & ~(x) & 0x80808080U)
#define hasmod32(x) haszero32(x ^ 0x25252525U)
#define align32(x) (((uintptr_t)x & 3) == 0)

#define haszero16(x) (((x) - 0x0101) & ~(x) & 0x8080)
#define hasmod16(x) haszero16(x ^ 0x2525)
#define align16(x) (((uintptr_t)x & 1) == 0)

typedef union DuoPtr {
    unsigned char *c;
    uint16_t *s;
    uint32_t *u;
    uint64_t *l;
} DuoPtr;

typedef union ConstDuoPtr {
    const unsigned char *c;
    const uint16_t *s;
    const uint32_t *u;
    const uint64_t *l;
} ConstDuoPtr;

void print(const char *format, ...) {
    static unsigned char buf[BUFFER_SIZE];
    DuoPtr _buf;
    ConstDuoPtr _fmt;
    _buf.c = buf;
    _fmt.c = (const unsigned char*)format;
    const unsigned char *str;
    double dbl;
    va_list args;
    va_start(args, format);
    while(1) {
        if(align64(_buf.c) && align64(_fmt.c)) {
            while(!haszero64(*_fmt.l) && !hasmod64(*_fmt.l)) *_buf.l++ = *_fmt.l++;
        } else if(align32(_buf.c) && align32(_fmt.c)) {
            while(!haszero32(*_fmt.u) && !hasmod32(*_fmt.u)) *_buf.u++ = *_fmt.u++;
        } else if(align16(_buf.c) && align16(_fmt.c)) {
            while(!haszero16(*_fmt.s) && !hasmod32(*_fmt.s)) *_buf.s++ = *_fmt.s++;
        }
        while(*_fmt.c && *_fmt.c != '%') *_buf.c++ = *_fmt.c++;
        if(!*_fmt.c) break;
        switch(*++_fmt.c) {
            case 'c':
                *_buf.c++ = va_arg(args, int);
                break;
            case 's':
                str = va_arg(args, const unsigned char*);
                while(*str) *_buf.c++ = *str++;
                break;
            case 'd': case 'i':
                _buf.c += parse_int(_buf.c, va_arg(args, int));
                break;
            case 'D': case 'I':
                _buf.c += parse_int(_buf.c, va_arg(args, long));
                break;
            case 'x':
                _buf.c += parse_uint_hex(_buf.c, va_arg(args, unsigned int));
                break;
            case 'X':
                _buf.c += parse_uint_hex(_buf.c, va_arg(args, unsigned long));
                break;
            case 'u':
                _buf.c += parse_uint(_buf.c, va_arg(args, unsigned int));
                break;
            case 'U':
                _buf.c += parse_uint(_buf.c, va_arg(args, unsigned long));
                break;
            case 'f': case 'F':
                dbl = va_arg(args, double);
                if(dbl) _buf.c += parse_double(_buf.c, dbl);
                else *_buf.c++ = '0';
                break; 
            case '%':
                *_buf.c++ = '%';
                break;
        }
        ++_fmt.c;
    }
    write(STDOUT, buf, _buf.c-buf);
    va_end(args);
}

#endif
