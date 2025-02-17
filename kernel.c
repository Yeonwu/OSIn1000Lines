//
// Created by ohye on 25. 1. 19.
//
#include "kernel.h"

typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
typedef uint32_t size_t;

extern char __bss[], __bss_end[], __stack_top[];
extern char __free_ram[], __free_ram_end[];

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

void handle_trap(struct trap_frame* f) {
    uint32_t scause = READ_CSR(scause);
    uint32_t stval = READ_CSR(stval);
    uint32_t user_pc = READ_CSR(sepc);

    PANIC("unexpected trap scause=%x, stval=%x, sepc=%x\n", scause, stval, user_pc);
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

struct process procs[PROCS_MAX];

struct process* create_process(uint32_t entry_point) {
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
    *--sp = entry_point; // ra;

    proc->pid = i;
    proc->state = PROC_RUNNABLE;
    proc->sp = (uint32_t) sp;

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

    WRITE_CSR(sscratch, (uint32_t) &next_proc->stack[sizeof(next_proc->stack)]);

    struct process* prev_proc = current_proc;
    current_proc = next_proc;
    switch_context(&prev_proc->sp, &next_proc->sp);
}

void delay() {
    for (int i = 0; i < 30000000; i++) {
        __asm__ ("nop");
    }
}

struct process *proc_a;
struct process *proc_b;

void proc_a_entry() {
    printf("start process a\n");
    while(1) {
        printf("pid %d A\n", proc_a->pid);
        delay();
        yield();
    }
}

void proc_b_entry() {
    printf("start process b\n");
    while(1) {
        printf("pid %d B\n", proc_b->pid);
        delay();
        yield();
    }
}

void kernel_main(void) {
    memset(__bss, 0, (size_t) __bss_end - (size_t) __bss);

    WRITE_CSR(stvec, (uint32_t) kernel_entry);

    idle_proc = create_process((uint32_t) NULL);
    idle_proc->pid = -1;
    current_proc = idle_proc;

    proc_a = create_process((uint32_t) proc_a_entry);
    proc_b = create_process((uint32_t) proc_b_entry);

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