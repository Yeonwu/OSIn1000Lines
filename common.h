//
// Created by ohye on 25. 1. 29.
//

#pragma once

#ifndef OSIN1000LINES_COMMON_H
#define OSIN1000LINES_COMMON_H

#define va_list __builtin_va_list
#define va_start __builtin_va_start
#define va_end __builtin_va_end
#define va_arg __builtin_va_arg

void printf(const char* format, ...);

#endif //OSIN1000LINES_COMMON_H
