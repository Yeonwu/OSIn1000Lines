//
// Created by ohye on 25. 1. 29.
//

#include "common.h"

void putchar(char ch);

void *memset(void *buf, char c, size_t n) {
    uint8_t *p = (uint8_t *) buf;

    while (n--)
        *p++ = c;

    return buf;
}

void *memcpy(void *dst, const void *src, size_t n) {
    uint8_t *p_src = (uint8_t *)src;
    uint8_t *p_dst = (uint8_t *)dst;

    while (n--)
        *p_dst++ = *p_src++;

    return dst;
}

char* strcpy(char *dst, const char *src) {
    char* d = dst;

    while (*src != '\0')
        *d++ = *src++;
    *d = '\0';

    return dst;
}

// s1 > s2 양수
// s1 < s2 음수
int strcmp(const char *s1, const char *s2) {
    while (*s1 != '\0' && *s2 != '\0') {
        if (*s1 != *s2) break;
        s1++;
        s2++;
    }
    return *(unsigned char *) s1 - *(unsigned char *) s2;
}

int str_same(const char *s1, const char *s2) {
    return strcmp(s1, s2) == 0;
}
int str_large(const char *s1, const char *s2) {
    return strcmp(s1, s2) > 0;
}
int str_small(const char *s1, const char *s2) {
    return strcmp(s1, s2) < 0;
}

void printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    char next;

    while (*format != '\0') {
        if (*format == '%') {
            next = *(format + 1);
            if (next == '\0') {
                putchar('%');
                continue;
            } else if (next == 'd') {
                int arg = va_arg(args, int);

                if (arg < 0) {
                    putchar('-');
                    arg *= -1;
                }

                int digits = 1;
                while (digits * 10 <= arg) digits *= 10;
                while (digits > 0) {
                    putchar('0' + arg / digits % 10);
                    digits /= 10;
                }
                format++;
            } else if (next == 's') {
                const char* arg = va_arg(args, const char *);
                while (*arg != '\0') {
                    putchar(*arg++);
                }
                format++;
            } else if (next == 'x') {
                const int arg = va_arg(args, int);
                for (int i = 7; i >= 0; i--) {
                    putchar("0123456789abcdef"[arg >> (i * 4) & 0xF]);
                }
                format++;
            } else if (next == '%') {
                putchar('%');
                format++;
            }
        } else {
            putchar(*format);
        }

        format++;
    }

    va_end(args);
}