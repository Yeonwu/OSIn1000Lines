cmake_minimum_required(VERSION 3.27)
project(OSIn1000Lines C)

# 컴파일러
set(CMAKE_C_COMPILER clang)
set(CMAKE_C_STANDARD 11)


# 경로
set(SRC "${CMAKE_SOURCE_DIR}/src")
set(BUILD "${CMAKE_SOURCE_DIR}/build")


# shell 관련 설정
add_executable(shell.elf
        ${SRC}/shell.c
        ${SRC}/user.c
        ${SRC}/common.c
)
# 헤더 파일
target_include_directories(shell.elf PRIVATE
        ${SRC}
)
#컴파일 옵션
target_compile_options(shell.elf PRIVATE
        -std=c11
        -O2 -g3 -Wall -Wextra
        --target=riscv32
        -ffreestanding -nostdlib
)
# 링킹 옵션
set(SHELL_LINKER_SCRIPT ${SRC}/user.ld)
set(SHELL_MAP "shell.map")
target_link_options(shell.elf PRIVATE
        --target=riscv32 -ffreestanding -nostdlib -fuse-ld=lld
        "-T${SHELL_LINKER_SCRIPT}"
        "-Wl,-Map=${SHELL_MAP}"
)
# shell.bin.o 생성
set(OBJCOPY llvm-objcopy)
add_custom_command(
        OUTPUT ${BUILD}/shell.bin.o
        COMMAND ${OBJCOPY} --set-section-flags .bss=alloc,contents -O binary shell.elf shell.bin && ${OBJCOPY} -Ibinary -Oelf32-littleriscv shell.bin shell.bin.o
        DEPENDS shell.elf
        COMMENT "Generate shell.bin.o from shell.elf"
)
add_custom_target(binobj ALL
        DEPENDS ${BUILD}/shell.bin.o
)


# kernel 관련 설정
add_executable(kernel.elf
        ${SRC}/kernel.c
        ${SRC}/common.c
        ${BUILD}/shell.bin.o
)
add_dependencies(kernel.elf binobj)

target_compile_options(kernel.elf PRIVATE
        -std=c11
        -O2 -g3 -Wall -Wextra
        --target=riscv32
        -ffreestanding -nostdlib
)

set(KERNEL_LINKER_SCRIPT ${SRC}/kernel.ld)
set(KERNEL_MAP "kernel.map")
target_link_options(kernel.elf PRIVATE
        --target=riscv32 -ffreestanding -nostdlib -fuse-ld=lld
        "-T${KERNEL_LINKER_SCRIPT}"
        "-Wl,-Map=${KERNEL_MAP}"
)