#!/bin/bash

set -xue

QEMU=qemu-system-riscv32

$QEMU -machine virt      \
      -bios scripts/opensbi-riscv32-generic-fw_dynamic.bin \
      -nographic         \
      -serial mon:stdio  \
      --no-reboot        \
      --kernel build/kernel.elf