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
    while(val) {
        buf[pos++] = (unsigned char)((val%10) + '0');
        val /= 10;
    }
    reverse(buf+sign, buf+pos-1);
    return pos;
}

static inline uint32_t parse_uint(unsigned char *buf, unsigned long val) {
    uint32_t pos = 0;
    while(val) {
        buf[pos++] = (unsigned char)((val%10) + '0');
        val /= 10;
    }
    reverse(buf, buf+pos-1);
    return pos;
}

uint32_t parse_uint_hex(unsigned char *buf, unsigned long val) {
    static unsigned char hexdigits[16] = "0123456789ABCDEF";
    uint32_t pos = 0;
    while(val) {
        buf[pos++] = hexdigits[val&15];
        val >>= 4;
    }
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

static inline double pow_10(uint32_t e) {
    return e == 0 ? 1 :
            e == 1 ? 10 :
            e == 2 ? 100 :
            e == 3 ? 1000 :
            e == 4 ? 10000 :
            e == 5 ? 100000 :
            e == 6 ? 1000000 :
            e == 7 ? 10000000 :
            e == 8 ? 100000000 : 1000000000;
}

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
        val /= 1000000000.0;
        exponent += 9;
    }
    u = val;
    uint32_t e = uintlog10(u);
    exponent += e;
    val /= pow_10(e);
    u = val;
    buf[pos++] = u%10 + '0';
    buf[pos++] = '.';
    val -= u;
    if(val) {
        val *= 1000000000.0;
        u = (uint32_t)val;
        val -= u;
        pos += parse_uint(buf+pos, u);
    }
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

void print(const char *format, ... ) {
    static unsigned char buf[BUFFER_SIZE];
    va_list args;
    va_start(args, format);
    uint32_t pos = 0;
    for(uint32_t i=0; format[i]; ++i) {
        if(pos >= BUFFER_SIZE-50) {
            write(STDOUT, buf, pos);
            pos = 0;
        }
        if(format[i] != '%') buf[pos++] = format[i];
        else {
            const unsigned char *str;
            switch(format[++i]) {
                case 'c':
                    buf[pos++] = va_arg(args, int);
                    break;
                case 's':
                    str = va_arg(args, const unsigned char*);
                    while(*str) buf[pos++] = *str++;
                    break;
                case 'd': case 'i':
                    pos += parse_int(buf+pos, va_arg(args, int));
                    break;
                case 'D': case 'I':
                    pos += parse_int(buf+pos, va_arg(args, long));
                    break;
                case 'x':
                    pos += parse_uint_hex(buf+pos, va_arg(args, unsigned int));
                    break;
                case 'X':
                    pos += parse_uint_hex(buf+pos, va_arg(args, unsigned long));
                    break;
                case 'u':
                    pos += parse_uint(buf+pos, va_arg(args, unsigned int));
                    break;
                case 'U':
                    pos += parse_uint(buf+pos, va_arg(args, unsigned long));
                    break;
                case 'f': case 'F':
                    pos += parse_double(buf+pos, va_arg(args, double));
                    break; 
                case '%':
                    buf[pos++] = '%';
                    break;
            }
        }
    }
    if(pos != 0) write(STDOUT, buf, pos);
    va_end(args);
}

#endif
