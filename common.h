//
// Created by ohye on 25. 1. 29.
//

#pragma once

#ifndef OSIN1000LINES_COMMON_H
#define OSIN1000LINES_COMMON_H

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wincompatible-library-redeclaration"

typedef int                     bool;
typedef unsigned char           uint8_t;
typedef unsigned short          uint16_t;
typedef unsigned int            uint32_t;
typedef unsigned long long int  uint64_t;
typedef uint32_t                size_t;
typedef uint32_t                paddr_t;
typedef uint32_t                vaddr_t;

#define true  1
#define false 0

#define NULL ((void*) 0)

#define align_up(value, align)      __builtin_align_up(value, align)
#define is_aligned(value, align)    __builtin_is_aligned(value, align)
#define offsetof(type, member)      ((size_t)(&(((type*)(0))->member)))

#define va_list     __builtin_va_list
#define va_start    __builtin_va_start
#define va_end      __builtin_va_end
#define va_arg      __builtin_va_arg

void* memset(void *buf, char c, size_t n);
void* memcpy(void *dst, void *src, size_t n);

char* strcpy(char *dst, const char *src);
int strcmp(const char *s1, const char *s2);

int str_same(const char *s1, const char *s2);
int str_large(const char *s1, const char *s2);
int str_small(const char *s1, const char *s2);

void printf(const char* format, ...);

#pragma clang diagnostic pop

#endif //OSIN1000LINES_COMMON_H