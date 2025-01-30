//
// Created by ohye on 25. 1. 29.
//

#include "common.h"

void putchar(char ch);

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
                while (digits * 10 < arg) digits *= 10;
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