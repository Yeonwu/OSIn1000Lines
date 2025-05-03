ENTRY(start)

SECTIONS {
    . = 0x1000000;

    /* machine code */
    .text :{
        KEEP(*(.text.start));
        *(.text .text.*);
    }

    /* read-only data */
    .rodata : ALIGN(4) {
        *(.rodata .rodata.*);
    }

    /* 초기화 값이 있는 데이터 */
    .data : ALIGN(4) {
        *(.data .data.*);
    }

    /* 시작 시 0으로 초기화 필요한 데이터 */
    .bss : ALIGN(4) {
        *(.bss .bss.* .sbss .sbss.*);

        . = ALIGN(16);
        . += 64 * 1024; /* 64KB */

        __stack_top = .;

        ASSERT(. < 0x1800000, "too large executable");
    }
}