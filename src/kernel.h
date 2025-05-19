//
// Created by ohye on 25. 1. 28.
//

#pragma once

#ifndef OSIN1000LINES_KERNEL_H
#define OSIN1000LINES_KERNEL_H

#include "common.h"

#define PANIC(fmt, ...)                                                         \
    do {                                                                        \
        printf("PANIC: %s:%d " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__);    \
        while(1) {}                                                             \
    } while(0)

#define READ_CSR(reg)                                           \
({                                                              \
    unsigned long __tmp;                                        \
    __asm__ __volatile__ ("csrr %0, " #reg : "=r"(__tmp));      \
    __tmp;                                                      \
})

#define WRITE_CSR(reg, value)                                   \
do {                                                            \
    uint32_t __tmp = (value);                                     \
    __asm__ __volatile__ ("csrw " #reg ", %0" :: "r"(__tmp));   \
} while(0)

struct sbiret {
    long error;
    long value;
};

struct trap_frame {
    uint32_t ra;
    uint32_t gp;
    uint32_t tp;
    uint32_t t0;
    uint32_t t1;
    uint32_t t2;
    uint32_t t3;
    uint32_t t4;
    uint32_t t5;
    uint32_t t6;
    uint32_t a0;
    uint32_t a1;
    uint32_t a2;
    uint32_t a3;
    uint32_t a4;
    uint32_t a5;
    uint32_t a6;
    uint32_t a7;
    uint32_t s0;
    uint32_t s1;
    uint32_t s2;
    uint32_t s3;
    uint32_t s4;
    uint32_t s5;
    uint32_t s6;
    uint32_t s7;
    uint32_t s8;
    uint32_t s9;
    uint32_t s10;
    uint32_t s11;
    uint32_t sp;
} __attribute__((packed));

#define PROCS_MAX 8

#define PROC_UNUSED   0
#define PROC_RUNNABLE 1
#define PROC_EXITED   2

struct process {
    int pid;
    int state;
    vaddr_t sp;
    uint8_t stack[8192];
    paddr_t page_table;
};

#define SATP_SV32 (1u << 31)
#define PAGE_V    (1 << 0)
#define PAGE_R    (1 << 1)
#define PAGE_W    (1 << 2)
#define PAGE_X    (1 << 3)
#define PAGE_U    (1 << 4)

#define USER_BASE 0x1000000

#define SSTATUS_SPIE (1<<5)

#define SCAUE_ECALL 8


void yield();
paddr_t alloc_pages(uint32_t n);

#define SECTOR_SIZE       512
#define VIRTQ_ENTRY_NUM   16
#define VIRTIO_DEVICE_BLK 2
#define VIRTIO_BLK_PADDR  0x10001000

#define VIRTIO_REG_MAGIC         0x00
#define VIRTIO_REG_VERSION       0x04
#define VIRTIO_REG_DEVICE_ID     0x08
#define VIRTIO_REG_FEATURES      0x10
#define VIRTIO_REG_QUEUE_SEL     0x30
#define VIRTIO_REG_QUEUE_NUM_MAX 0x34
#define VIRTIO_REG_QUEUE_NUM     0x38
#define VIRTIO_REG_QUEUE_ALIGN   0x3c
#define VIRTIO_REG_QUEUE_PFN     0x40
#define VIRTIO_REG_QUEUE_READY   0x44
#define VIRTIO_REG_QUEUE_NOTIFY  0x50
#define VIRTIO_REG_DEVICE_STATUS 0x70

#define VIRTIO_STATUS_ACK       1
#define VIRTIO_STATUS_DRIVER    2
#define VIRTIO_STATUS_DRIVER_OK 4
#define VIRTIO_STATUS_FEAT_OK   8

#define VIRTQ_DESC_F_NEXT          1
#define VIRTQ_DESC_F_WRITE         2

#define VIRTQ_AVAIL_F_NO_INTERRUPT 1

#define VIRTIO_BLK_F_GEOMETRY 4

#define VIRTIO_BLK_T_IN  0
#define VIRTIO_BLK_T_OUT 1

struct virtio_blk_config {
    uint64_t capacity;
    uint32_t size_max;
    uint32_t seg_max;
    struct virtio_blk_geometry {
        uint16_t cylinders;
        uint8_t heads;
        uint8_t sectors;
    }__attribute__((packed)) geometry;
    uint32_t blk_size;
    struct virtio_blk_topology {
        // # of logical blocks per physical block (log2)
        uint8_t physical_block_exp;
        // offset of first aligned logical block
        uint8_t alignment_offset;
        // suggested minimum I/O size in blocks
        uint16_t min_io_size;
        // optimal (suggested maximum) I/O size in blocks
        uint32_t opt_io_size;
    }__attribute__((packed)) topology;
    uint8_t writeback;
    uint8_t unused0[3];
    uint32_t max_discard_sectors;
    uint32_t max_discard_seg;
    uint32_t discard_sector_alignment;
    uint32_t max_write_zeroes_sectors;
    uint32_t max_write_zeroes_seg;
    uint8_t write_zeroes_may_unmap;
    uint8_t unused1[3];
}__attribute__((packed));

#define VIRTIO_DEVICE_CONFIG (*((volatile struct virtio_blk_config*) (VIRTIO_BLK_PADDR + 0x100)))

// Virtqueue Descriptor area entry.
struct virtq_desc {
    uint64_t addr;
    uint32_t len;
    uint16_t flags;
    uint16_t next;
} __attribute__((packed));

// Virtqueue Available Ring.
struct virtq_avail {
    uint16_t flags;
    uint16_t index;
    uint16_t ring[VIRTQ_ENTRY_NUM];
} __attribute__((packed));

// Virtqueue Used Ring entry.
struct virtq_used_elem {
    uint32_t id;
    uint32_t len;
} __attribute__((packed));

// Virtqueue Used Ring.
struct virtq_used {
    uint16_t flags;
    volatile uint16_t index;
    struct virtq_used_elem ring[VIRTQ_ENTRY_NUM];
} __attribute__((packed));

// Virtqueue.
struct virtio_virtq {
    struct virtq_desc descs[VIRTQ_ENTRY_NUM];
    struct virtq_avail avail;
    struct virtq_used used __attribute__((aligned(PAGE_SIZE)));
    uint16_t queue_index;
    volatile uint16_t *used_index;
    uint16_t last_used_index;
} __attribute__((packed));

// Virtio-blk request.
struct virtio_blk_req {
    uint32_t type;
    uint32_t reserved;
    uint64_t sector;

    uint8_t data[512];

    uint8_t status;
} __attribute__((packed));

#endif //OSIN1000LINES_KERNEL_H
