//
// Created by ohye on 25. 1. 19.
//
#include "kernel.h"

typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
typedef uint32_t size_t;

extern char __kernel_base[];
extern char __bss[], __bss_end[], __stack_top[];
extern char __free_ram[], __free_ram_end[];
extern char _binary_shell_bin_start[], _binary_shell_bin_size[];

struct sbiret sbi_call(long arg0, long arg1, long arg2, long arg3, long arg4, long arg5, long fid, long eid) {
    register long a0 __asm__("a0") = arg0;
    register long a1 __asm__("a1") = arg1;
    register long a2 __asm__("a2") = arg2;
    register long a3 __asm__("a3") = arg3;
    register long a4 __asm__("a4") = arg4;
    register long a5 __asm__("a5") = arg5;
    register long a6 __asm__("a6") = fid;
    register long a7 __asm__("a7") = eid;

    __asm__ __volatile__("ecall"
    : "=r"(a0), "=r"(a1)
    : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5), "r"(a6), "r"(a7)
    : "memory");

    return (struct sbiret){.error = a0, .value = a1};
}

void putchar(char ch) {
    sbi_call(ch, 0, 0, 0, 0, 0, 0, 1);
}

long getchar(void) {
    struct sbiret ret = sbi_call(0, 0, 0, 0, 0, 0, 0, 2);
    return ret.error;
}

__attribute__((naked))
__attribute__((aligned(4)))
void kernel_entry(void) {
    __asm__ __volatile__(
            "csrrw sp, sscratch, sp\n"  // 현재 스택포인터 값을 sscratch에 저장하고 sscratch에 저장된 커널스택 스택포인터 값을 가져온다.
            "addi sp, sp, -4 * 31\n"    // 현재 스택 포인터 -= 4 * 31 (32바이트 크기의 스택 확보)

            "sw ra,  4 * 0(sp)\n"       // 스택에 현재 레지스터 값 전부 저장
            "sw gp,  4 * 1(sp)\n"
            "sw tp,  4 * 2(sp)\n"
            "sw t0,  4 * 3(sp)\n"
            "sw t1,  4 * 4(sp)\n"
            "sw t2,  4 * 5(sp)\n"
            "sw t3,  4 * 6(sp)\n"
            "sw t4,  4 * 7(sp)\n"
            "sw t5,  4 * 8(sp)\n"
            "sw t6,  4 * 9(sp)\n"
            "sw a0,  4 * 10(sp)\n"
            "sw a1,  4 * 11(sp)\n"
            "sw a2,  4 * 12(sp)\n"
            "sw a3,  4 * 13(sp)\n"
            "sw a4,  4 * 14(sp)\n"
            "sw a5,  4 * 15(sp)\n"
            "sw a6,  4 * 16(sp)\n"
            "sw a7,  4 * 17(sp)\n"
            "sw s0,  4 * 18(sp)\n"
            "sw s1,  4 * 19(sp)\n"
            "sw s2,  4 * 20(sp)\n"
            "sw s3,  4 * 21(sp)\n"
            "sw s4,  4 * 22(sp)\n"
            "sw s5,  4 * 23(sp)\n"
            "sw s6,  4 * 24(sp)\n"
            "sw s7,  4 * 25(sp)\n"
            "sw s8,  4 * 26(sp)\n"
            "sw s9,  4 * 27(sp)\n"
            "sw s10, 4 * 28(sp)\n"
            "sw s11, 4 * 29(sp)\n"

            "csrr a0, sscratch\n"       // 저장해둔 스택 포인터 값을 다시 불러와서 스택에 저장
            "sw a0, 4 * 30(sp)\n"

            "addi a0, sp, 4 * 31\n"     // 커널 스택 값을 다시 복원해서 sscratch에 저장
            "csrw sscratch, a0\n"

            "mv a0, sp\n"               // a0 레지스터에 현재 스택 포인터값을 넣어 handle_trap 호출 시 인자로 넘겨준다.
            // handle_trap 함수는 struct trap frame* 타입의 인자를 하나 받는다.
            // 스택에 값들을 넣고 난 다음의 스택 포인터 주소를 넘겨주어 struct trap frame*로 사용한다.
            "call handle_trap\n"        // handle_trap 함수 호출


            "lw ra,  4 * 0(sp)\n"       // 스택에 넣어두었던 값을 다시 레지스터로 가져온다.
            "lw gp,  4 * 1(sp)\n"
            "lw tp,  4 * 2(sp)\n"
            "lw t0,  4 * 3(sp)\n"
            "lw t1,  4 * 4(sp)\n"
            "lw t2,  4 * 5(sp)\n"
            "lw t3,  4 * 6(sp)\n"
            "lw t4,  4 * 7(sp)\n"
            "lw t5,  4 * 8(sp)\n"
            "lw t6,  4 * 9(sp)\n"
            "lw a0,  4 * 10(sp)\n"
            "lw a1,  4 * 11(sp)\n"
            "lw a2,  4 * 12(sp)\n"
            "lw a3,  4 * 13(sp)\n"
            "lw a4,  4 * 14(sp)\n"
            "lw a5,  4 * 15(sp)\n"
            "lw a6,  4 * 16(sp)\n"
            "lw a7,  4 * 17(sp)\n"
            "lw s0,  4 * 18(sp)\n"
            "lw s1,  4 * 19(sp)\n"
            "lw s2,  4 * 20(sp)\n"
            "lw s3,  4 * 21(sp)\n"
            "lw s4,  4 * 22(sp)\n"
            "lw s5,  4 * 23(sp)\n"
            "lw s6,  4 * 24(sp)\n"
            "lw s7,  4 * 25(sp)\n"
            "lw s8,  4 * 26(sp)\n"
            "lw s9,  4 * 27(sp)\n"
            "lw s10, 4 * 28(sp)\n"
            "lw s11, 4 * 29(sp)\n"
            "lw sp, 4 * 30(sp)\n"

            "sret\n"                  // kernel_entry가 호출되었던 지점으로 복귀
            );
}
uint32_t virtio_reg_read32(unsigned offset) {
    return *((volatile uint32_t*) (VIRTIO_BLK_PADDR + offset));
}

uint64_t virtio_reg_read64(unsigned offset) {
    return *((volatile uint64_t*) (VIRTIO_BLK_PADDR + offset));
}

void virtio_reg_write32(unsigned offset, uint32_t value) {
    *((volatile uint32_t*) (VIRTIO_BLK_PADDR + offset)) = value;
}

void virtio_reg_fetch_and_or32(unsigned offset, uint32_t value) {
    virtio_reg_write32(offset, virtio_reg_read32(offset) | value);
}

paddr_t blk_req_paddr;
struct virtio_blk_req* blk_req;
uint32_t blk_capacity;
struct virtio_virtq* blk_req_virtq;

struct virtio_virtq* virtq_init(uint16_t index) {
    printf("virtio: initalizing virtq\n");

    paddr_t virtq_paddr = alloc_pages(align_up(sizeof(struct virtio_virtq), PAGE_SIZE) / PAGE_SIZE);
    struct virtio_virtq* vq = (struct virtio_virtq*) virtq_paddr;
    vq->queue_index = index;
    vq->used_index = (volatile uint16_t *) &vq->used.index;

    // Select the queue writing its index (first queue is 0) to QueueSel.
    virtio_reg_write32(VIRTIO_REG_QUEUE_SEL, vq->queue_index);

    // Check if the queue is not already in use: read QueuePFN, expecting a returned value of zero (0x0).
    if (virtio_reg_read32(VIRTIO_REG_QUEUE_PFN) != 0) {
        printf("virtio: QueuePFN is %d\n", virtio_reg_read32(VIRTIO_REG_QUEUE_PFN));
        PANIC("virtio: QueuePFN value is invalid.\n");
    }

    // Read maximum queue size (number of elements) from QueueNumMax. If the returned value is zero (0x0) the queue is not available.
    uint32_t queue_max_num = virtio_reg_read32(VIRTIO_REG_QUEUE_NUM_MAX);
    if (queue_max_num == 0) {
        PANIC("virtio: QueueNumMax value is zero. The queue is not available.\n");
    }

    // Allocate and zero the queue pages in contiguous virtual memory, aligning the Used Ring to an optimal boundary (usually page size).
    // The driver should choose a queue size smaller than or equal to QueueNumMax.

    // Since we zero the page in alloc_page(), we don't need to repeat it.
    if (VIRTQ_ENTRY_NUM > queue_max_num) {
        PANIC("virtio: Queue size is bigger than QueueMaxNum. (VIRTQ_ENTRY_NUM=%d, queue_max_num=%d)\n", VIRTQ_ENTRY_NUM, queue_max_num);
    }

    // Notify the device about the queue size by writing the size to QueueNum.
    virtio_reg_write32(VIRTIO_REG_QUEUE_NUM, VIRTQ_ENTRY_NUM);

    // Notify the device about the used alignment by writing its value in bytes to QueueAlign.
    // Set the value to zero, because we didn't use alignment in virtq struct.
    virtio_reg_write32(VIRTIO_REG_QUEUE_ALIGN, 0);

    // Write the physical number of the first page of the queue to the QueuePFN register.
    virtio_reg_write32(VIRTIO_REG_QUEUE_PFN, virtq_paddr);

    return vq;
}

void virtio_blk_init(void) {
    // 4.2.3.1.1 Driver Requirements: Device Initialization
    // The driver MUST start the device initialization by reading and checking values from MagicValue and Version.

    if (virtio_reg_read32(VIRTIO_REG_MAGIC) != 0x74726976) {
        PANIC("virtio: invalid virtio magic number.\n");
    }

    uint32_t device_version = virtio_reg_read32(VIRTIO_REG_VERSION);
    if (device_version != 2) {
        printf("virtio: device version %d\n", device_version);
        if (device_version == 1) {
            printf("virtio: device is legacy.\n");
        } else {
            PANIC("virtio: invaild virtio device version.\n");
        }
    }

    // if both values are valid, it MUST read DeviceID and if its value is zero (0x0) MUST abort initialization.
    uint32_t device_id = virtio_reg_read32(VIRTIO_REG_DEVICE_ID);
    if (device_id == 0) {
        PANIC("virtio: device id is zero.\n");
    }
    // Since we are using only Block device, which device id is 2, other device id causes panic.
    if (device_id != VIRTIO_DEVICE_BLK) {
        PANIC("virtio: device is not block device.\n");
    }

    // Reset the device.
    // According to 4.2.2 MMIO Device Register Layout, Set Status register value 0x0 to reset the device.
    virtio_reg_write32(VIRTIO_REG_DEVICE_STATUS, 0);

    // Set the ACKNOWLEDGE status bit: the guest OS has noticed the device.
    // See 2.1 Device Status Field.
    virtio_reg_fetch_and_or32(VIRTIO_REG_DEVICE_STATUS, VIRTIO_STATUS_ACK);

    // Set the DRIVER status bit: the guest OS knows how to drive the device.
    virtio_reg_fetch_and_or32(VIRTIO_REG_DEVICE_STATUS, VIRTIO_STATUS_DRIVER);

    // Read device feature bits, and write the subset of feature bits understood by the OS and driver to the device.
    // During this step the driver MAY read (but MUST NOT write) the device-specific configuration fields to check that
    // it can support the device before accepting it.
    uint32_t feature = virtio_reg_read32(VIRTIO_REG_FEATURES);

    virtio_reg_write32(VIRTIO_REG_FEATURES, feature);

    // Set the FEATURES_OK status bit. The driver MUST NOT accept new feature bits after this step.
    virtio_reg_fetch_and_or32(VIRTIO_REG_DEVICE_STATUS, VIRTIO_STATUS_FEAT_OK);

    // Re-read device status to ensure the FEATURES_OK bit is still set:
    // otherwise, the device does not support our subset of features and the device is unusable.
    if (!(virtio_reg_read32(VIRTIO_REG_DEVICE_STATUS) & VIRTIO_STATUS_FEAT_OK)) {
        PANIC("virtio: device status features ok bit is not set.\n");
    }

    // Perform device-specific setup, including discovery of virtqueues for the device, optional per-bus setup,
    // reading and possibly writing the device’s virtio configuration space, and population of virtqueues.
    blk_req_virtq = virtq_init(0);

    // Set the DRIVER_OK status bit. At this point the device is “live”.
    virtio_reg_write32(VIRTIO_REG_DEVICE_STATUS, VIRTIO_STATUS_DRIVER_OK);
    printf("virtio: device ready.\n");

    blk_capacity = VIRTIO_DEVICE_CONFIG.capacity * SECTOR_SIZE;
    printf("virtio: device capacity %d bytes.\n", (int)blk_capacity);

    blk_req_paddr = alloc_pages(align_up(sizeof(*blk_req), PAGE_SIZE) / PAGE_SIZE);
    blk_req = (struct virtio_blk_req*) blk_req_paddr;
}

void virtq_kick(struct virtio_virtq* vq, int desc_index) {
    vq->avail.ring[vq->avail.index % VIRTQ_ENTRY_NUM] = desc_index;
    vq->avail.index++;

    __sync_synchronize();
    virtio_reg_write32(VIRTIO_REG_QUEUE_NOTIFY, vq->queue_index);
    vq->last_used_index++;
}

bool virtq_is_busy(struct virtio_virtq* vq) {
    return vq->last_used_index != *vq->used_index;
}

void set_blk_req_header(struct virtio_virtq* vq) {
    vq->descs[0].addr = blk_req_paddr;
    vq->descs[0].len = sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint64_t); // type, reserved, sector
    vq->descs[0].flags = VIRTQ_DESC_F_NEXT;
    vq->descs[0].next = 1;
}

void set_blk_req_data(struct virtio_virtq* vq, bool writable) {
    vq->descs[1].addr = blk_req_paddr + offsetof(struct virtio_blk_req, data);
    vq->descs[1].len = SECTOR_SIZE;
    vq->descs[1].flags = VIRTQ_DESC_F_NEXT | (writable ? VIRTQ_DESC_F_WRITE : 0);
    vq->descs[1].next = 2;
}

void set_blk_req_status(struct virtio_virtq* vq) {
    vq->descs[2].addr = blk_req_paddr + offsetof(struct virtio_blk_req, status);
    vq->descs[2].len = sizeof (uint8_t);
    vq->descs[2].flags = VIRTQ_DESC_F_WRITE;
}

int read_disk(void *buf, uint64_t sector) {
    blk_req->sector = sector;
    blk_req->type = VIRTIO_BLK_T_IN;

    struct virtio_virtq* vq = blk_req_virtq;

    set_blk_req_header(vq);
    set_blk_req_data(vq, true);
    set_blk_req_status(vq);

    virtq_kick(vq, 0);
    while (virtq_is_busy(vq))
        ;

    if (blk_req->status != 0) {
        printf("virtio: failed to read. status=%d\n", blk_req->status);
        return -1;
    }

    memcpy(buf, blk_req->data, SECTOR_SIZE);
    return 0;
}

int write_disk(void *buf, uint64_t sector) {
    blk_req->sector = sector;
    blk_req->type = VIRTIO_BLK_T_OUT;
    memcpy(blk_req->data, buf, SECTOR_SIZE);

    struct virtio_virtq* vq = blk_req_virtq;

    set_blk_req_header(vq);
    set_blk_req_data(vq, true);
    set_blk_req_status(vq);

    virtq_kick(vq, 0);
    while(virtq_is_busy(vq))
        ;

    if (blk_req->status != 0) {
        printf("virtio: failed to write. status=%d\n", blk_req->status);
        return -1;
    }

    return 0;
}

paddr_t alloc_pages(uint32_t n) {
    static paddr_t next_paddr = (paddr_t) __free_ram;

    paddr_t curr_paddr = next_paddr;
    next_paddr += (n * PAGE_SIZE);

    if (next_paddr > (paddr_t) __free_ram_end) {
        PANIC("out of memory: using %x, available: %x, tried to allocate %x.", \
        curr_paddr, (paddr_t)__free_ram_end, next_paddr);
    }

    memset((void*) curr_paddr, 0, n * PAGE_SIZE);

    return curr_paddr;
}

void map_page(uint32_t* table1, vaddr_t vaddr, paddr_t paddr, uint32_t flags) {
    if (!is_aligned(vaddr, PAGE_SIZE))
        PANIC("unaligned vaddr %x\n", vaddr);

    if (!is_aligned(paddr, PAGE_SIZE))
        PANIC("unaligned paddr %x\n", paddr);

    uint32_t vpn1 = (vaddr >> 22) & 0x3FF;
    if ((table1[vpn1] & PAGE_V) == 0) {
        paddr_t table2_page_addr = alloc_pages(1);
        table1[vpn1] = ((table2_page_addr / PAGE_SIZE) << 10) | PAGE_V;
    }

    uint32_t vpn2 = (vaddr >> 12) & 0x3FF;
    paddr_t* table2 = (paddr_t*) ((table1[vpn1] >> 10) * PAGE_SIZE);
    table2[vpn2] = ((paddr / PAGE_SIZE) << 10) | PAGE_V | flags;
}

__attribute__((naked))
void switch_context(uint32_t *prev_sp, uint32_t *next_sp) {
    __asm__ __volatile__ (
            "addi sp, sp, -4 * 13\n"
            "sw  ra ,  0 * 4(sp)\n"
            "sw  s0 ,  1 * 4(sp)\n"
            "sw  s1 ,  2 * 4(sp)\n"
            "sw  s2 ,  3 * 4(sp)\n"
            "sw  s3 ,  4 * 4(sp)\n"
            "sw  s4 ,  5 * 4(sp)\n"
            "sw  s5 ,  6 * 4(sp)\n"
            "sw  s6 ,  7 * 4(sp)\n"
            "sw  s7 ,  8 * 4(sp)\n"
            "sw  s8 ,  9 * 4(sp)\n"
            "sw  s9 , 10 * 4(sp)\n"
            "sw  s10, 11 * 4(sp)\n"
            "sw  s11, 12 * 4(sp)\n"

            "sw sp, (a0)\n"
            "lw sp, (a1)\n"

            "lw  ra ,  0 * 4(sp)\n"
            "lw  s0 ,  1 * 4(sp)\n"
            "lw  s1 ,  2 * 4(sp)\n"
            "lw  s2 ,  3 * 4(sp)\n"
            "lw  s3 ,  4 * 4(sp)\n"
            "lw  s4 ,  5 * 4(sp)\n"
            "lw  s5 ,  6 * 4(sp)\n"
            "lw  s6 ,  7 * 4(sp)\n"
            "lw  s7 ,  8 * 4(sp)\n"
            "lw  s8 ,  9 * 4(sp)\n"
            "lw  s9 , 10 * 4(sp)\n"
            "lw  s10, 11 * 4(sp)\n"
            "lw  s11, 12 * 4(sp)\n"
            "addi sp, sp, 4 * 13\n"

            "ret"
            );
}

__attribute__((naked))
void user_entry(void) {
    __asm__ __volatile__(
            "csrw sepc, %[sepc]     \n"
            "csrw sstatus, %[sstatus]   \n"
            "sret"
            :
            : [sepc] "r" (USER_BASE),
              [sstatus] "r" (SSTATUS_SPIE)
            );
}

struct process procs[PROCS_MAX];

struct process* create_process(const void *image, size_t image_size) {
    uint8_t i;
    struct process* proc = NULL;

    for (i = 0; i < PROCS_MAX; i++) {
        if (procs[i].state == PROC_UNUSED) {
            proc = &procs[i];
            break;
        }
    }

    if (proc == NULL) PANIC("There is no unused process slot.");

    uint32_t *sp = (uint32_t*) &proc->stack[sizeof(proc->stack)];

    *--sp = 0; // s11;
    *--sp = 0; // s10;
    *--sp = 0; // s9;
    *--sp = 0; // s8;
    *--sp = 0; // s7;
    *--sp = 0; // s6;
    *--sp = 0; // s5;
    *--sp = 0; // s4;
    *--sp = 0; // s3;
    *--sp = 0; // s2;
    *--sp = 0; // s1;
    *--sp = 0; // s0;
    *--sp = (uint32_t) user_entry; // ra;

    paddr_t page_table = alloc_pages(1);
    for (
            paddr_t addr = (paddr_t)__kernel_base;
            addr < (paddr_t)__free_ram_end;
            addr += PAGE_SIZE
        ) {
        map_page(
            (uint32_t *)page_table,
            addr,
            addr,
            PAGE_R | PAGE_W | PAGE_X
            );
    }

    for (uint32_t offset = 0; offset < image_size; offset += PAGE_SIZE) {
        paddr_t page = alloc_pages(1);

        size_t remaining = image_size - offset;
        size_t copy_size = PAGE_SIZE <= remaining ? PAGE_SIZE : remaining;

        memcpy((void *)page, image + offset, copy_size);
        map_page(
                (uint32_t *)page_table,
                USER_BASE + offset,
                page,
                PAGE_U | PAGE_R | PAGE_W | PAGE_X
                );
    }

    map_page(
            (uint32_t*)page_table,
            VIRTIO_BLK_PADDR,
            VIRTIO_BLK_PADDR,
            PAGE_R | PAGE_W
            );

    proc->pid = i;
    proc->state = PROC_RUNNABLE;
    proc->sp = (uint32_t) sp;
    proc->page_table = page_table;

    return proc;
}

struct process *current_proc;
struct process *idle_proc;

void yield() {
    struct process *next_proc = idle_proc;
    for (int i = 1; i <= PROCS_MAX; i++) {
        struct process *proc = &procs[(current_proc->pid + i) % PROCS_MAX];
        if (proc->state == PROC_RUNNABLE && proc->pid > 0) {
            next_proc = proc;
            break;
        }
    }

    if (next_proc == current_proc) return;

    __asm__ __volatile__ ("sfence.vma");
    WRITE_CSR(satp, (SATP_SV32 | ((uint32_t) next_proc->page_table / PAGE_SIZE)));
    __asm__ __volatile__ ("sfence.vma");

    WRITE_CSR(sscratch, (uint32_t) &next_proc->stack[sizeof(next_proc->stack)]);

    struct process* prev_proc = current_proc;
    current_proc = next_proc;
    switch_context(&prev_proc->sp, &next_proc->sp);
}

void handle_syscall(struct trap_frame* f) {
    switch (f->a3) {
        case SYS_PUTCHAR:
            putchar(f->a0);
            break;
        case SYS_GETCHAR:
            while(1) {
                long ch = getchar();
                if (ch >= 0) {
                    f->a0 = ch;
                    break;
                }

                yield();
            }
            break;
        case SYS_EXIT:
            printf("process %d exited\n", current_proc->pid);
            current_proc->state = PROC_EXITED;
            yield();
            PANIC("unreachable");
            break;
        default:
            PANIC("unexpected syscall a3=%x\n", f->a3);
    }
}

void handle_trap(struct trap_frame* f) {
    uint32_t scause = READ_CSR(scause);
    uint32_t stval = READ_CSR(stval);
    uint32_t user_pc = READ_CSR(sepc);

    if (scause == SCAUE_ECALL) {
        handle_syscall(f);
        user_pc += 4;
    } else {
        PANIC("unexpected trap scause=%x, stval=%x, sepc=%x\n", scause, stval, user_pc);
    }

    WRITE_CSR(sepc, user_pc);
}

void kernel_main(void) {
    memset(__bss, 0, (size_t) __bss_end - (size_t) __bss);

    WRITE_CSR(stvec, (uint32_t) kernel_entry);

    virtio_blk_init();
    char buf[SECTOR_SIZE + 1] = {0, };
    if (read_disk(buf, 0) < 0) PANIC("Failed to read disk.");
    printf("Disk content: %s\n", buf);

    strcpy(buf, "hello from kernel!!\n");
    if (write_disk(buf, 0) < 0) PANIC("Failed to write disk.");
    printf("Write finished\n");


    idle_proc = create_process(NULL, 0);
    idle_proc->pid = -1;
    current_proc = idle_proc;

    create_process(_binary_shell_bin_start, (size_t) _binary_shell_bin_size);

    yield();
    PANIC("switched to idle process");
}

__attribute__((section(".text.boot")))
__attribute__((naked))
void boot(void) {
    __asm__ __volatile__(
            "mv sp, %[stack_top]\n" // Set the stack pointer
            "j kernel_main\n"       // Jump to the kernel main function
            :
            : [stack_top] "r" (__stack_top) // Pass the stack top address as %[stack_top]
    );
}