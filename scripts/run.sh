#!/bin/bash

set -xue

QEMU=qemu-system-riscv32

pwd

$QEMU -machine virt -nographic -serial mon:stdio --no-reboot \
      -bios scripts/opensbi-riscv32-generic-fw_dynamic.bin \
      -d unimp,guest_errors,int,cpu_reset -D qemu.log \
      --kernel build/kernel.elf \
      -drive id=drive0,file=lorem.txt,format=raw,if=none \
      -device virtio-blk-device,drive=drive0,bus=virtio-mmio-bus.0


      # -global virtio-mmio.force-legacy=false \