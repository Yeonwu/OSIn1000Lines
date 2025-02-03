//
// Created by ohye on 25. 1. 28.
//

#pragma once

#ifndef OSIN1000LINES_KERNEL_H
#define OSIN1000LINES_KERNEL_H

#define PANIC(fmt, ...)                                                         \
    do {                                                                        \
        printf("PANIC: %s:%d " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__);    \
        while(1) {}                                                             \
    } while(0)

struct sbiret {
    long error;
    long value;
};

#endif //OSIN1000LINES_KERNEL_H
