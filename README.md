# OSIn1000Lines

참고: https://github.com/nuta/operating-system-in-1000-lines

## 시작하기

### 환경

- OS: Ubuntu 23.10
- IDE: CLion

### 설치
가상의 CPU를 애뮬레이트하기 위해 QEMU를 사용한다.

QEMU 설치 중 아래와 같은 오류가 발생하였다.

```shell
$sudo apt update

E: The repository 'http://ports.ubuntu.com/ubuntu-ports mantic Release' no longer has a Release file.
N: Updating from such a repository can't be done securely, and is therefore disabled by default.
```

Ubuntu 23.10 EOL일자(2024.7.11)가 도래함으로 인해 지원이 종료되어 Repository에 접근이 막혔다. 정석적인 해결방법은 Ubuntu 버전을 업그레이드 하는 것이지만, 시간이 너무 오래걸리고 귀찮으므로 보안 취약성을 무릅쓰고 구버전 Repository를 그냥 사용하기로 결정했다.

`/etc/apt/source.list`의 원래 내용을 주석처리하고 아래 내용으로 변경하였다.
```shell
deb http://old-releases.ubuntu.com/ubuntu/ ㅡmantic main restricted universe multiverse
deb http://old-releases.ubuntu.com/ubuntu/ ㅡmantic-updates main restricted universe multiverse
deb http://old-releases.ubuntu.com/ubuntu/ ㅡmantic-security main restricted universe multiverse

```
참고: https://help.ubuntu.com/community/EOLUpgrades


이후 정상적으로 QEMU 설치를 마쳤다.
```shell
$sudo apt install qemu-system-riscv32
$curl -LO https://github.com/qemu/qemu/raw/v8.0.4/pc-bios/opensbi-riscv32-generic-fw_dynamic.bin

$qemu-system-riscv32 --version

QEMU emulator version 8.0.4 (Debian 1:8.0.4+dfsg-1ubuntu3.23.10.5)
Copyright (c) 2003-2022 Fabrice Bellard and the QEMU Project developers
```

OpenSBI는 BIOS랑 비슷한 거라고 생각하면 된다.

## RISC-V 기초

### 어셈블리 기초

어셈블리어는 기계어와 거의 1대1대응 관계이다. 
```
addi a0, a1, 123
```
대부분 1줄에 명령 1개가 들어있다. 예시에서는 `addi`로, `a1`레지스터의 값과 `123`을 더한 뒤 `a0`에 저장하는 명령이다. `addi`는 opcode라고도 부른다.

레지스터는 CPU가 바로바로 사용할 수 있는 굉장히 빠른 메모리라고 생각하면 된다. CPU가 메모리에 있는 값을 사용하기 위해서는 먼저 레지스터로 값을 불러와야한다. 이후 연산을 수행하고, 그 결과를 레지스터나 메모리에 저장한다. 다음은 RISC-V에서 자주 사용하는 레지스터들이다.

| Register      | ABI Name (alias) | Description                                     |
|---------------|------------------|-------------------------------------------------|
| `pc`          | `pc`             | Program counter (where the next instruction is) |
| `x0`          | `zero`           | Hardwired zero (always reads as zero)           |
| `x1`          | `ra`             | Return address                                  |
| `x2`          | `sp`             | Stack pointer                                   |
| `x5` - `x7`   | `t0` - `t2`      | Temporary registers                             |
| `x8`          | `fp`             | Stack frame pointer                             |
| `x10` - `x11` | `a0` - `a1`      | Function arguments/return values                |
| `x12` - `x17` | `a2` - `a7`	     | Function arguments                              |
| `x18` - `x27` | `s0` - `s11`     | Temporary registers saved across calls          |
| `x28` - `x31` | `t3` - `t6`	     | Temporary registers                             |

사실 레지스터는 그 용도가 정해져 있지는 않다. 원하는 용도로 사용해도 잘 동작하지만, 다른 소프트웨어와의 호환성을 위해 저렇게 용도가 정의되어있다.

#### 메모리 접근

레지스터는 빠르지만 개수가 제한되어있다. 따라서 다음과 같은 흐름으로 데이터를 다룬다.

메인 메모리 > 레지스터 > (연산) > 레지스터 > 메인 메모리

이때 레지스터에서 메모리로, 메모리에서 레지스터로 데이터를 옮기는데 사용하는 명령어가 lw(load word, 메모리 > 레지스터), sw(save word, 레지스터 > 메모리)이다.

```asm
// 괄호 안의 값은 메모리 주소이다.

lw a0 (a1) // a1 주소에 저장된 word(32-bit 아키텍처이므로 32bit 크기)를 a0 레지스터에 저장한다. C로 치면 a0 = *a1;
sw a0 (a1) // a0 레지스터에 저장된 word를 a1 주소에 저장한다. C로 치면 *a1 = *a0;
```

#### 브랜치 명령어

브랜치 명령어는 If, For, While 등 제어문을 구현한다. 

```asm
bnez a0, <label> // bnez: Branch if Not Equal to Zero, a0 레지스터 값이 0이 아니면 <label>로 이동한다.
    // a0 == 0일 경우
label:
    // a0 != 0일 경우
```
`beq`(Branch if EQual), `bls`(Branch if LeSs than)과 같은 명령어도 있다.

#### 함수 호출

`jal`(Jump And Link)와 `ret`(RETurn)을 사용해 함수 호출을 구현한다.

```asm
li a0, 123 // 함수 인자로 123을 넘기기 위해 a0 레지스터에 123값을 저장한다. 
jal ra, <label> // label 위치에 정의된 함수를 실행한다. 리턴주소를 ra 레지스터에 저장한다.

<label>
    addi a0, a0, 1 // a0 += 1

    ret // 리턴 주소로 돌아간다. 리턴값은 a0 레지스터에 있다.
```

컨벤션에 따라 `a0`~`a7` 레지스터로 함수 인자를 넘겨주며, `a0` 레지스터에 리턴값을 저장한다. 

#### 스택

FIFO구조의 메모리로, 아래방향으로 확장된다. 함수 호출, 로컬 변수 등에 사용된다. `sp` 레지스터에 스택의 top 주소를 저장한다.

Push, Pop 연산은 다음과 같이 구현할 수 있다. 
```asm
addi sp, sp, -4 // sp = sp - 4
sw a0, sp       // *sp = a0

lw sp, a0       // a0 = *sp
addi sp, sp, 4  // sp = sp + 4
```

#### CPU 모드 & Privileged Instructions

CPU는 모드에 따라 갖는 실행 권한이 달라진다. RISC-V에서는 다음 3가지 모드가 있다.

| 모드     | 권한                      |
|--------|-------------------------|
| M-mode | Open SBI(BIOS)가 동작하는 모드 |
| S-mode | 커널 모드                   |
| U-mode | 유저 모드                   |

유저 모드에서 실행할 수 없는 명령들을 privileged instruction 이라고 부른다.

여기서 사용하는 privileged instruction은 다음과 같다.

| 명령어               | 설명                                        | 의사코드               |
|-------------------|-------------------------------------------|--------------------|
| csrr rd, csr      | csr read, csr값을 읽어 rd에 저장한다.              | rd = csr           |
| csrw csr, rs      | csr write, rs에 저장된 값을 csr에 저장한다.          | csr = rs           |
| csrrw rd, csr, rs | csrr, csrw를 한번에                           | rd = csr; csr = rs |
| sret              | trap 핸들러로부터 돌아온다.(프로그램 카운터, CPU모드 등을 복구)  |                    |
| sfence.vma        | Translation Lookaside Buffer(TLB)를 초기화한다. |                    |

CSR(Control and Status Register)는 CPU 세팅을 저장하는 레지스터다.

### 인라인 어셈블리

C코드에 어셈블리를 삽입할 수 있다. 인라인 어셈블리를 사용하면 어셈블리 파일을 따로 작성하는 것에 비해 다음 2가지 장점이 있다.
- C에서 사용하고 있는 변수를 바로 어셈블리로 가져올 수 있고, 어셈블리에서 C언어 변수에 값을 할당할 수 있다. 
- 레지스터 할당을 컴파일러가 알아서 해주기 때문에, 어떤 레지스터를 사용중인지 일일히 기억하며 코드를 짤 필요가 없다.

#### 문법

인라인 어셈블리 문법은 다음과 같다.

```c
__asm__ __volatile__ ("assembly": 출력 : 입력: clobbered register)
```

- `__asm__`: 인라인 어셈블리임을 나타낸다.
- `__volatile__`: 컴파일러가 어셈블리 최적화를 하지 않도록 한다.
- `"assembly`: 어셈블리 명령어이다.
- `출력`: 어셈블리 명령어의 결과값을 저장할 C 변수이다.
- `입력`: 어셈블리 명령어에 입력값으로 들어갈 C 변수이다.
- `clobbered register`: 컴파일러가 해당 레지스터를 사용하지 않는다. 특정 레지스터 값을 보존해야 할 때 사용한다.

#### 예시

```c
uint32_t value;
__asm__  __volatile__ ("csrr %0, sepc" : "=r"(value));
```

위 예시는 `sepc` CSR 값을 `csrr`명령어를 사용해 읽어와 `value`에 저장한다. `%0`은 value에 해당하며, `"=r"`은 임의의 레지스터에 값을 읽어오겠다는 뜻이다.
위 코드를 어셈블리로 바꿔보면 이렇게 생겼다.
```asm
csrr a0, sepc
sw (&value) a0 // a0 레지스터 값을 value 변수 주소에 저장한다.
```

```c
__asm__ __volatile__ ("csrw sscratch, %0" : : "r"(123));
```

위 예시는 `sscratch` CSR에 123값을 대입한다. `"r"`은 해당값을 갖는 레지스터하는 의미로 쓰인다. 위 코드를 어셈블리로 바꿔보면 이렇게 생겼다.
```asm
li a0, 123
csrw sscratch, a0
```

"r", "=r"을 constraint 라고 부른다. 인라인 어셈블리를 컴파일러가 어셈블리로 바꿀 때 어떻게 바꿀지를 지정한다.

## 부팅 

컴퓨터 전원이 켜지면 가장 먼저 BIOS가 실행된다. 하드웨어를 초기화하고, 시작화면을 표시하고, OS를 디스크에서 읽어와 메모리에 올리고 실행시킨다.

QEMU 가상머신에서 BIOS의 기능을 하는 것이 OpenSBI이다.

Supervisor Binary Interface, SBI는 운영체제 커널에게 펌웨어가 제공하는 API 스펙이고, 가장 유명한 구현체가 Open SBI이다.

SBI 스펙은 [Github](https://github.com/riscv-non-isa/riscv-sbi-doc/releases)에 공개되어 있으며, 시리얼 포트 등 디버그 콘솔에 문자 표시, 다시시작/종료, 타이머 등의 기능이 정의되어있다.

### OpenSBI 부팅하기

`run.sh`파일을 아래와 같이 작성하고 실행하자.

```shell
#!/bin/bash

set -xue

QEMU=qemu-system-riscv32

$QEMU -machine virt -bios default -nographic -serial mon:stdio --no-reboot
```

QEMU는 가상머신 시작 시 다양한 옵션을 설정할 수 있다. 여기에 사용한 옵션은 다음과 같다.

- `-machine virt`: 가상환경에서 머신을 시작한다. `-machine ?` 옵션으로 다른 머신들을 확인할 수 있다.
- `-bios default`: 기본 펌웨어(OpenSBI)를 사용한다.
- `-nographic`: GUI 없이 실행한다.
- `serial mon:stdio`: QEMU의 표준 입출력을 가상머신의 시리얼 포트에 연결한다. `mon:`은 Ctrl+A, C를 눌러 QEMU 모니터로 전환할 수 있게 해준다.
- `--no-reboot`: 가상머신에서 충돌이 일어나도 재부팅하지 않게한다. 디버깅에 유용하다.

`run.sh`을 실행하면 아래와 같은 배너가 출력된다.

```shell
OpenSBI v1.2
   ____                    _____ ____ _____
  / __ \                  / ____|  _ \_   _|
 | |  | |_ __   ___ _ __ | (___ | |_) || |
 | |  | | '_ \ / _ \ '_ \ \___ \|  _ < | |
 | |__| | |_) |  __/ | | |____) | |_) || |_
  \____/| .__/ \___|_| |_|_____/|____/_____|
        | |
        |_|

Platform Name             : riscv-virtio,qemu
Platform Features         : medeleg
Platform HART Count       : 1
Platform IPI Device       : aclint-mswi
Platform Timer Device     : aclint-mtimer @ 10000000Hz
Platform Console Device   : uart8250
Platform HSM Device       : ---
...
```

OpenSBI 버전, 플랫폼 이름, HART 개수(CPU 코어) 등 디버깅에 필요한 정보들이 출력된다.

키보드를 눌러도 아무 일도 없는데, QEMU의 표준 입출력이 가상머신의 시리얼 포트에 연결되어 있기 때문이다. OpenSBI가 입력된 문자를 받고 있지만, 이를 읽어서 사용하는 프로그램(OS 등)이 없어 버려지고 있는 것이다.

Ctrl+A, C를 눌러 QEMU 디버그 콘솔(QEMU 모니터)로 전환할 수 있다. q를 누르면 QEMU에서 나갈 수 있다.

```shell
QEMU 8.0.4 monitor - type 'help' for more information
(qemu) q
```

### 링커 스크립트

링커 스크립트는 실행파일의 메모리 구조를 결정하는 파일이다. 링커는 함수와 변수에 메모리 주소를 할당할 때 링커 스크립트에 정의된 구조를 따른다.

`kernel.ld`라는 이름의 파일을 만들고 아래와 같이 작성하자.

```linkerscript
/* 엔트리 포인트는 boot 함수이다. */
ENTRY(boot)

SECTIONS {
    /* 해당 주소에서 시작한다. .은 현재 주소를 나타낸다.*/
    . = 0x80200000;
    /* text 영역 */
    .text :{
    /* .text.boot 영역은 항상 맨 처음에 배치된다. */
        KEEP(*(.text.boot));
        /* .text 영역과 .text로 시작하는 모든 영역을 이곳에 배치한다. */
        *(.text .text.*);
    }
    /* rodata 영역, 4Byte로 나누어 떨어지도록 맞춘다. */
    .rodata : ALIGN(4) {
        *(.rodata .rodata.*);
    }
    /* data 영역, 4Byte로 나누어 떨어지도록 맞춘다. */
    .data : ALIGN(4) {
        *(.data .data.*);
    }
    /* bss 영역, 4Byte로 나누어 떨어지도록 맞춘다. */
    .bss : ALIGN(4) {
        /* bss 영역 시작 주소를 __bss에 저장한다. */
        __bss = .;
        *(.bss .bss.* .sbss .sbss.*);
        /* bss 영역 끝 주소를 __bss_end에 저장한다. */
        __bss_end = .;
    }

    . = ALIGN(4);
    /* 현재 주소를 128 * 1024 Byte(128 KB)만큼 증가시켜 스택 영역을 확보한다.*/
    . += 128 * 1024;
    /* 스택 영역 시작 주소를 __stack_top에 저장한다. 스택은 아래로 자란다. (push 연산시 top은 감소) */
    __stack_top = .;
}
```

### 간단한 커널
`kernel.c` 파일을 만들고 아래와 같이 작성하자.
```c
typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
typedef uint32_t size_t;

extern char __bss[], __bss_end[], __stack_top[]; 
// 링커 스크립트에 정의된 변수를 가져온다.
// extern: 외부 변수를 가져오겠다는 의미이다. 이 경우에는 링커 스크립트에 정의된 외부변수이다.
// char: 메모리 주소가 중요하기 때문에 포인터 타입은 중요하지 않다.
// []: __bss는 bss 영역의 시작점에 저장된 값을 의미한다. __bss[]로 작성해야 주소를 가져올 수 있다.


void *memset(void *buf, char c, size_t n) {
    uint8_t *p = (uint8_t *) buf;
    while (n--)
        *p++ = c;
    return buf;
}

void kernel_main(void) {
    // bss 영역의 값을 0으로 초기화해준다.
    memset(__bss, 0, (size_t) __bss_end - (size_t) __bss);

    for (;;);
}

__attribute__((section(".text.boot")))
__attribute__((naked))
// __attribute__은 GCC 컴파일러가 제공하는 확장기능이다(표준은 아니다). 함수속성, 변수속성, 타입속성이 존재한다.
// boot 함수에 적용된 속성은 두가지인데, section("section name")속성은 대상을 section name 영역에 위치시킨다.
// 여기서는 함수를 .text.boot 영역에 위치시킨다.
// naked 속성은 함수에 불필요한 명령(예를 들어 리턴문 등)을 추가하지 않게 한다. 
// 이를 통해 인라인 어셈블리 내용과 함수 내용을 일치시킨다.
void boot(void) {
    // boot 함수가 링커 스크립트에서 엔트리 포인트로 설정되었으므로 이 함수가 가장 먼저 실행된다.
    __asm__ __volatile__(
        "mv sp, %[stack_top]\n" // 스택 포인터 레지스터에 링커 스크립트에서 설정한 값을 넣는다.
        "j kernel_main\n"       // kernel_main 함수로 점프한다.
        :
        : [stack_top] "r" (__stack_top) // %[stack_top]에 __stack_top 변수 값을 전달한다.
    );
}
```

### 커널 컴파일 & 실행

run.sh를 다음과 같이 변경하자.
```shell
#!/bin/bash
set -xue

QEMU=qemu-system-riscv32

# 
CC=clang
CFLAGS="-std=c11 -O2 -g3 -Wall -Wextra --target=riscv32 -ffreestanding -nostdlib"

# Build the kernel
$CC $CFLAGS -Wl,-Tkernel.ld -Wl,-Map=kernel.map -o kernel.elf kernel.c

# Start QEMU
$QEMU -machine virt -bios default -nographic -serial mon:stdio --no-reboot -kernel kernel.elf
```

clang 컴퍼일러에 설정하는 옵션은 다음과 같다.

- `-std=c11`: C11을 사용한다.
- `-O2`: O2 수준으로 최적화한다.
- `-g3`: 디버그 정보를 최대한으로 생성한다.
- `-Wall`: 주요한 Warning을 활성화한다.
- `-Wextra`: 추가적인 Warning을 활성화한다.
- `--target=riscv32`: 32bit risc-v cpu 대상으로 컴파일한다.
- `-ffreestanding`: 개발 환경에서 제공하는 표준 라이브러리를 사용하지 않는다.
- `-nostdlib`: 표준 라이브러리를 링크하지 않는다.
- `-Wl,-Tkernel.ld`: kernel.ld 링크 스크립트를 사용한다.
- `-Wl,-Map=kernel.map`: 링커가 메모리를 할당한 결과를 Map 파일로 저장한다.

`-Wl`은 컴파일러가 아닌 링커에게 넘겨주는 옵션이다.

run.sh를 실행하면 이전과 다를 것 없이 아무런 입력도, 출력도 없다. 작성한 커널이 정상적으로 실행되는지 확인하기 위해서는 QEMU의 디버깅 기능을 사용하면 된다.
QEMU 모니터로 전환한 후, `info register`를 입력하면 현재 레지스터에 저장된 값이 출력된다. 그 중 pc, 프로그램 카운터 레지스터 값을 확인하자.

```
QEMU 8.0.2 monitor - type 'help' for more information
(qemu) info registers

CPU#0
 V      =   0
 pc       8020004c
 ...
```
8020004c 주소에서 프로그램 카운터가 멈춰있다. 값은 환경에 따라 다를 수 있다.
다음으로 llvm-objdump를 사용해 kernel.elf 파일을 분석해보자.

```
kernel.elf:     file format elf32-littleriscv

Disassembly of section .text:

// boot 함수 부분이다.
80200000 <boot>:
80200000: 37 05 22 80   lui     a0, 524832
80200004: 13 05 05 05   addi    a0, a0, 80
80200008: 2a 81         mv      sp, a0
8020000a: 6f 00 a0 01   j       0x80200024 <kernel_main> // kernel_main을 실행한다.
8020000e: 00 00         unimp

...

80200024 <kernel_main>:
80200024: 37 05 20 80   lui     a0, 524800
80200028: 13 05 05 05   addi    a0, a0, 80
8020002c: b7 05 20 80   lui     a1, 524800
80200030: 93 85 05 05   addi    a1, a1, 80
80200034: 33 86 a5 40   sub     a2, a1, a0
80200038: 11 ca         beqz    a2, 0x8020004c <.LBB1_3>
8020003a: b3 05 b5 40   sub     a1, a0, a1

...

8020004c <.LBB1_3>:
8020004c: 01 a0         j       0x8020004c <.LBB1_3> // 프로그램 카운터는 여기 위치해있다. 계속해서 같은 위치로 점프하는 무한 루프이므로, 커널이 잘 실행되고 있음을 알 수 있다.
```

## Hello world!

### ecall

콘솔에 문자를 출력하기 위해서 SBI에서 제공하는 API인 `ecall`를 사용한다.

```c
// 인자를 받아 `ecall`을 호출한다. 
struct sbiret sbi_call(long arg0, long arg1, long arg2, long arg3, long arg4, long arg5, long fid, long eid) {
    register long a0 __asm__("a0") = arg0; // a0 레지스터에 arg0 값을 넣는다. register와 __asm__("a0") 키워드를 사용해 a0 변수를 a0 레지스터에 저장한다.
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
```

`ecall`에 대한 스펙은 다음과 같이 정의되어 있다.

> 챕터 3
> 모든 SBI 함수는 하나의 바이너리 인코딩을 공유하며, 이는 SBI 확장들이 호환되기 쉽도록 한다. SBI 스펙은 다음 관례를 따른다.
>
> - ECALL은 Supervisor와 SEE(Supervisor Execution Environment) 사이의 제어 전환 명령으로 사용된다.
> - a7 레지스터는 SBI Extension ID(EID)를 인코딩한다.
> - a6 레지스터는 SBI funciton ID(FID)를 인코딩한다.
> - 호출자는 a0, a1을 제외한 모든 레지스터를 SBI 호출간에 보존해야 한다.
> - SBI 함수는 a0에 에러 코드를, a1에 리턴값을 넣어 리턴해야 한다.
> 
> -- "RISC-V Supervisor Binary Interface Specification" v2.0-rc1

ecall이 호출되는 순간, CPU의 모드는 커널 모드(S-Mode, Supervisor)에서 OpenSBI 모드(M-Mode, SEE)로 전환된다. OpenSBI가 호출된 SBI 함수 실행을 끝내면 다시 커널 모드로 돌아온다.

콘솔에 문자를 출력하기 위해서는 console_putchar 함수를 호출하면 된다.

> 5.2. 확장: Console Putchar (EID #0x01)
```c 
long sbi_console_putchar(int ch)
```
> ch에 들어있는 데이터를 디버그 콘솔에 쓴다.
> sbi_console_getchar()와 달리, 전송중인 문자가 남아있거나 터미널이 준비되지 않았다면 블록된다.
> 만약 콘솔이 존재하지 않는다면 문자는 버려진다.
> 실행에 성공할 경우 0을 리턴하며, 실패할 경우 음수(구현에 따라 다름)을 리턴한다.
> -- "RISC-V Supervisor Binary Interface Specification" v2.0-rc1

이에 맞게 작성한 putchar 함수는 다음과 같다.
```c
void putchar(char ch) {
    sbi_call(ch, 0, 0, 0, 0, 0, 0, 1);
}
```

hello world!를 출력하기 위해 kernel_main 함수를 수정하자.
```c
void kernel_main(void) {
    memset(__bss, 0, (size_t) __bss_end - (size_t) __bss);

    const char* s = "\n\nhello world!\n";
    for (int i = 0; s[i] != '\0'; i++) {
        putchar(s[i]);
    }

    for (;;) {
        __asm__ __volatile__("wfi");
    }
}
```

run.sh를 실행하면 hello world!가 출력된다.

hello world!가 출력되는 과정을 좀 더 자세히 살펴보자.

1. 커널이 ecall 명령어를 실행한다. CPU는 OpenSBI가 부팅시 세팅해놓은 M-mode trap handler로 점프한다.(mtvec 레지스터) 
2. 레지스터를 저장한 후, [C언어로 작성된 trap handler](https://github.com/riscv-software-src/opensbi/blob/0ad866067d7853683d88c10ea9269ae6001bcf6f/lib/sbi/sbi_trap.c#L263)가 실행된다.
3. EID에 따라 대응하는 [SBI함수](https://github.com/riscv-software-src/opensbi/blob/0ad866067d7853683d88c10ea9269ae6001bcf6f/lib/sbi/sbi_ecall_legacy.c#L63C2-L65)가 호출된다.
4. 8250 UART [드라이버](https://github.com/riscv-software-src/opensbi/blob/0ad866067d7853683d88c10ea9269ae6001bcf6f/lib/utils/serial/uart8250.c#L77)가 QEMU에 문자를 전송한다.
5. QEMU의 8250 UART 에뮬레이션 구현체가 문자를 받아 표준 출력으로 전송한다.
6. 터미널 에뮬레이터가 문자를 표시한다.

### printf

아래와 같이 간단한 printf 함수를 구현할 수 있다. 가변 인자를 받기 위한 `va_list, va_start, va_arg, va_end`는 C 표준 라이브러리인 stdargs.h에 정의되어 있지만, 여기서는 Clang에 빌트인된 `__builtin_va_...`를 사용한다.

`common.h`
```c
#pragma once

#ifndef OSIN1000LINES_COMMON_H
#define OSIN1000LINES_COMMON_H

#define va_list __builtin_va_list
#define va_start __builtin_va_start
#define va_end __builtin_va_end
#define va_arg __builtin_va_arg

void printf(const char* format, ...);

#endif //OSIN1000LINES_COMMON_H
```

`common.c`
```c
#include "common.h"

void putchar(char ch);

void printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    char next;

    while (*format != '\0') {
        if (*format == '%') {
            next = *(format + 1);
            if (next == '\0') {
                putchar('%');
                continue;
            } else if (next == 'd') {
                int arg = va_arg(args, int);

                if (arg < 0) {
                    putchar('-');
                    arg *= -1;
                }

                int digits = 1;
                while (digits * 10 < arg) digits *= 10;
                while (digits > 0) {
                    putchar('0' + arg / digits % 10);
                    digits /= 10;
                }
                format++;
            } else if (next == 's') {
                const char* arg = va_arg(args, const char *);
                while (*arg != '\0') {
                    putchar(*arg++);
                }
                format++;
            } else if (next == 'x') {
                const int arg = va_arg(args, int);
                for (int i = 7; i >= 0; i--) {
                    putchar("0123456789abcdef"[arg >> (i * 4) & 0xF]);
                }
                format++;
            } else if (next == '%') {
                putchar('%');
                format++;
            }
        } else {
            putchar(*format);
        }

        format++;
    }

    va_end(args);
}
```

이후 kernel.c에서 common.h를 include하고, run.sh에서 컴파일 대상에 common.c를 추가한 후 실행한다.

`run.sh`
```shell
...
$CC $CFLAGS -Wl,-Tkernel.ld -Wl,-Map=kernel.map -o kernel.elf \
  kernel.c common.c
...
```

`kernel.c`
```c
#include common.h

...

void kernel_main(void) {
    memset(__bss, 0, (size_t) __bss_end - (size_t) __bss);

    const char* s = "\n\nhello world!\n";
    printf("console: %s", s);
    printf("date: %d %d %d\n", 2025, 1, 0);
    printf("%x\n", 0x1234abcd);

    for (;;) {
        __asm__ __volatile__("wfi");
    }
}

...
```

## C 표준 라이브러리

몇개의 타입과 메모리 및 문자열 조작 함수들을 직접 구현한다.

먼저 다음의 타입들을 `common.h`에 선언한다.

```c
typedef int                     bool;
typedef unsigned char           uint8_t;
typedef unsigned short          uint16_t;
typedef unsigned int            uint32_t;
typedef unsigned long long int  uint64_t;
typedef uint32_t                size_t;
typedef uint32_t                paddr_t;
typedef uint32_t                vaddr_t;
```

`size_t`는 c언어 표준 라이브러리에서 배열, 객체 등의 크기를 나타내기 위해 사용하는 타입이다. `sizeof`가 리턴하는 값, 메모리를 할당하는 `malloc(n)`에서 n이 `size_t` 타입이다.
`
unsigned int`를 사용하지 않고 `size_t`를 따로 사용하는 이유는 아키텍처 간 확장성과 성능을 모두 고려하기 위해서이다. 일반적으로 `unsigned int`와 `pointer`타입은 32bit으로 크기가 같다. 하지만 `unsigned int`가 16bit, `pointer`타입이 32bit일 경우에 `size_t`로 `unsigned int`를 사용하게 되면 32bit 범위를 전부 커버할 수 없다. 

그럼 아예 넉넉하게 `size_t`로 `long`을 사용하면 안될까? 이 경우 성능적으로 문제가 있다. 예를 들어 `unsigned int`, `pointer`가 32bit, `long`이 64bit인 아키텍처를 생각해보자. 일반적인 32bit 아키텍처는 64bit을 구현하기 위해 32bit씩 2번 연산한다. 따라서 일괄적으로 `long`을 사용할 경우 몇몇 아키텍처에서 성능적으로 떨어지게 된다.

추가로 코드를 읽기 쉬워지는 장점도 있다. 프로그래머는 `size_t` 변수를 볼 때 n바이트 크기를 나타내고 있음을 짐작할 수 있다.
[출처](https://www.embedded.com/why-size_t-matters/)

`paddr_t`는 물리 메모리 주소를 나타내기 위한 타입이다.

`vaddr_t`는 가상 메모리 주소를 나타내기 위한 타입이다.

다음으로 몇가지 유용한 메크로를 선언한다.
```c
#define true  1
#define false 0

#define NULL ((void*) 0)

#define align_up(value, align)      __builtin_align_up(value, align)
#define is_aligned(value, align)    __builtin_is_aligned(value, align)
#define offsetof(type, member)      ((int)(&(((type*)(0))->member)))
```

`NULL`은 널 포인터로 사용되는 값이다. `0` 대신 `(void*) 0`을 사용하는 이유는 해당 값이 '포인터'임을 확실히 하기 위해서이다. 코드를 올바로 작성했다면 `x = 0`과 같이 `0`을 `NULL` 대신 사용해도 똑같이 동작한다. 하지만 예컨데,

```c
int *x = &a;
x = NULL;
```
처럼 `int* x`를 널 포인터로 초기화하려 했으나 실수로 아래와 같이 적었다 가정해보자.

```c
int *x = &a;
*x = NULL;
```

이 경우, `*x`는 `int` 타입 변수인 `a`에 대한 참조이므로 우변에 `int` 타입이 주어져야한다. 하지만 포인터 타입인 `NULL`이 주어졌기 때문에 컴파일러를 에러를 발생시키고, 실수를 예방할 수 있다.
[출처](https://stackoverflow.com/questions/61718737/why-is-there-a-null-in-the-c-language)

`align_up`은 `value`를 가장 작은 `align`의 배수로 올림한다. 예를 들어 `align_up(15, 8)`은 `16`이다. `align`은 2의 거듭제곱수이다.

`is_aligned`는 `value`가 `align`의 배수인지 확인한다. 마찬가지로 `align`은 2의 거듭제곱수이다.

`offsetof`는 주어진 구조체 타입 `type`에서 `member`의 위치를 확인한다. 즉, 구조체의 시작점으로부터 몇 바이트가 떨어져 있는지를 확인한다. 매크로는 다음과 같은 원리로 동작한다.

```c
(
    (size_t)(             // 4. int로 캐스팅하여 멤버가 구조체 시작점(0)에서 떨어진 거리를 구한다.
        &(                // 3. 구한 멤버의 주소값을 구한다.
            ((type*)(0))  // 1. 0 포인터를 주어진 구조체 타입의 포인터로 캐스팅한다.
            ->member      // 2. 해당 포인터에서 주어진 멤버에 접근한다.
        )
    )
)
```
물론 널 포인터를 참조하면 에러가 발생한다. 하지만 위 경우는 상수 표현식이기 때문에 실제 값에 접근하지 않으므로 동작한다.
[출처1](https://stackoverflow.com/questions/7897877/how-does-the-c-offsetof-macro-work)
[출처2](http://port70.net/~nsz/c/c11/n1570.html#6.6p9)

마지막으로 메모리 및 문자열 조작 함수를 선언한다. `str_same`, `str_large`, `str_small`은 c 표준 라이브러리에는 없지만, `strcmp`의 사용이 직관적이지 못하기 때문에 추가하였다. 예를 들어 문자열 `s1`과  `s2`가 같을 때 `!strcmp(s1, s2)`가 참이다.

```c
void* memset(void *buf, char c, size_t n);
void* memcpy(void *dst, void *src, size_t n);

char* strcpy(char *dst, const char *src);
int strcmp(const char *s1, const char *s2);

int str_same(const char *s1, const char *s2);
int str_large(const char *s1, const char *s2);
int str_small(const char *s1, const char *s2);
```

선언한 함수는 `common.c`에 아래와 같이 구현되어 있다. 이 중 `memset` 함수는 `kernel.c`에 구현해두었던 코드를 옮겨왔다.

```c
void *memset(void *buf, char c, size_t n) {
    uint8_t *p = (uint8_t *) buf;

    while (n--)
        *p++ = c;

    return buf;
}

void *memcpy(void *dst, void *src, size_t n) {
    uint8_t *p_src = (uint8_t *)src;
    uint8_t *p_dst = (uint8_t *)dst;

    while (n--)
        *p_dst++ = *p_src++;

    return dst;
}

char* strcpy(char *dst, const char *src) {
    char* d = dst;

    while (*src != '\0')
        *d++ = *src++;
    *d = '\0';

    return dst;
}

// s1 > s2 양수
// s1 < s2 음수
int strcmp(const char *s1, const char *s2) {
    while (*s1 != '\0' && *s2 != '\0') {
        if (*s1 != *s2) break;
        s1++;
        s2++;
    }
    return *(unsigned char *) s1 - *(unsigned char *) s2;
}

int str_same(const char *s1, const char *s2) {
    return strcmp(s1, s2) == 0;
}
int str_large(const char *s1, const char *s2) {
    return strcmp(s1, s2) > 0;
}
int str_small(const char *s1, const char *s2) {
    return strcmp(s1, s2) < 0;
}
```

## 커널 패닉

커널 패닉은 커널이 회복 불가능한 오류가 발생했을 때 일어난다. 윈도우나 리눅스의 블루 스크린과 같은 컨셉이다. 

`kernel.h`에 정의한 다음 `PANIC` 매크로를 사용해 커널 패닉이 일어났을 때 디버깅 정보를 출력하고, 프로그램을 멈출 수 있다.

```c
#define PANIC(fmt, ...)                                                         \
    do {                                                                        \
        printf("PANIC: %s:%d " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__);    \
        while(1) {}                                                             \
    } while(0)
```

함수가 아닌 매크로로 정의한 이유는 `__FILE__`, `__LINE__`을 사용하기 위해서이다. 만약 함수로 작성했다면 `PANIC`이 일어난 곳이 아닌, `PANIC`함수가 정의된 파일과 라인이 출력된다.

`do {...} while(0)`은 C언어에서 여러줄로 구성된 매크로를 작성할 때 사용하는 관례이다. 그냥 `{...}`로 묶을 수도 있겠지만, 이 경우 아래 코드와 같이 `if`문과의 부작용이 발생할 수 있다.

```c
#define SWAP(x, y) { \
    int tmp = x;     \
    x = y;           \
    y = tmp;         \
}

if (...) 
    SWAP(x, y);
else
    ...
// 위 코드는 아래와 같이 해석되며, 에러를 발생시킨다.

if (...)
    {int tmp = x;x = y;y = tmp;}; // if문 다음에 세미콜론이 붙는다!
else
    ...
```

줄 마지막에 `\`이 붙었기 때문에 컴파일러가 매크로를 해석할 때 줄바꿈을 무시하고 한 줄로 취급한다.

`__VA_ARGS__`는 매크로에서 가변인자를 사용할 수 있도록 해주는 매크로이다. 앞에 붙은 "##"은 가변인자가 주어지지 않았을 때 앞의 콤마를 지워주는 역할을 한다.

`kernel.c`의 `kernel_main` 함수에서 커널 패닉을 일으켜보자.
```c
void kernel_main(void) {
    memset(__bss, 0, (size_t) __bss_end - (size_t) __bss);

    const char* s = "\n\nhello world!\n";
    printf("console: %s", s);
    
    PANIC("AHHHHHH!");
    
    printf("date: %d %d %d\n", 2025, 1, 0);
    printf("%x\n", 0x1234abcd);

    for (;;) {
        __asm__ __volatile__("wfi");
    }
}
```
출력:
```
hello world!
PANIC: kernel.c:40 AHHHHHH!
```

패닉 이후의 코드는 실행되지 않는걸 확인할 수 있다.

## 예외

예외처리는 커널이 다양한 이벤트에 대응할 수 있도록하는 CPU의 기능이다. CPU가 잘못된 메모리 접근, 잘못된 명령어 사용, 시스템 콜 등을 마주쳤을 때 예외처리가 일어난다. 예외처리는 `try-catch`문과 비슷하게 작동하는데, 정상적으로 프로그램을 실행하다 커널이 처리해야할 이벤트가 일어나면 예외를 발생시켜 기존 콘텍스트에서 예외처리 콘텍스트로 넘어간다. `try-catch`와 다른 점은 예외처리가 끝나면 예외가 발생한 지점으로 돌아가 이어서 프로그램을 실행할 수 있다는 점이다.

예외는 커널모드에서도 일어날 수 있으며, 그런 예외는 치명적인 커널 버그로 이어진다. 만약 QEMU가 비정상적으로 다시 시작하거나, 커널이 의도한데로 작동하지 않는다면 예외가 일어났을 확률이 높다. 때문에 예외가 발생했을 때 커널 패닉을 일으켜 디버깅을 용이하게 하기 위해 예외처리를 먼저 구현했다.

### 예외 라이프 사이클

RISC-V에서 예외는 다음과 같이 처리된다.

1. CPU가 `medeleg` 레지스터를 확인해 어떤 모드에서 예외를 처리할지 결정한다. OpenSBI에 의해 U-Mode(유저), S-Mode(커널)에서 발생한 예외를 S-Mode(커널)에서 정의된 핸들러가 처리하도록 설정되어 있다.
2. CPU가 자신의 상태를 여러 CSR(Control and Status Register)에 저장한다. 대표적으로는 예외의 종류를 저장하는 `scause`, 예외 종류에 따른 추가 정보(예외 발생 메모리 주소 등)를 저장하는 `stval`, 예외가 발생한 프로그램 카운터 위치를 저장하는 `sepc`, 예외가 발생한 모드(U-Mode/S-Mode 등)을 저장하는 `sstatus`등이 있다.
3. 프로그램 카운터 값이 `stvec` 레지스터에 저장된 값으로 바뀐다. (따라서 `stvec` 레지스터에 예외 핸들러의 메모리 주소를 저장해두어야 한다.)
4. 예외 핸들러가 예외를 처리한다.

예외 핸들러가 호출되면 다음과 같이 동작한다.
1. 현재 레지스터들을 저장한다.
2. 예외를 처리한다.
3. 저장해둔 레지스터 값을 불러와 예외 처리 전과 같은 상태로 되돌린다.
4. `sret`명령을 호출해 예외가 일어났던 지점으로 복귀한다.

#### `ret` vs `sret`, `mret`, `uret`

`ret`은 함수 실행이 끝나고 호출된 지점으로 복귀하는 명령어이다. 실제로 존재하는 명령어가 아닌 의사 명령어로, CPU는 `ret`이 호출되면 `jalr x0, 0(x1)`을 실행한다. 이는 `x1` 레지스터에 저장된 함수 호출 주소로 점프하라는 뜻이다.

반면 `sret`, `mret`, `uret`(이하 `xret`)은 실제 명령어이다. RISC-V privileged document에 의하면, 
> `xret`은 x-Mode에서 발생한 트랩에서 리턴하기 위해 사용된다. `xret`이 호출되었을 때 `xPP` 값이 `y`이고, `x IE`가 `x PIE`로 설정되었다고 가정하자. 권한 모드는 `y-Mode`로 변경되고, `x PIE`는 1로, `xPP`는 `U`로(U-Mode가 지원되지 않을 경우 M으로) 변경된다.

[출처](https://stackoverflow.com/questions/73103031/what-is-the-difference-between-mret-and-ret-instruction-in-machine-mode)

### 예외 핸들러 
예외 핸들러에 사용한 매크로와 타입을 `kernel.h`에 작성한다. `READ_CSR`, `WRITE_CSR` 매크로는 CSR을 조작하기 위한 매크로이다. `trap_frame` 구조체는 예외 핸들러가 저장한 레지스터 값을 읽어오기 위해 사용하는 구조체이다. 
```c
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
```

아래와 같이 `kernel.c`에 예외 핸들러를 작성한다. 위에서 설명한 예외 핸들러의 동작과 똑같이 동작한다.
```c
__attribute__((naked))
__attribute__((aligned(4)))
void kernel_entry(void) {
    __asm__ __volatile__(
            "csrw sscratch, sp\n"       // 현재 스택 포인터 값 저장
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
```
`kernel_entry`가 호출하는 `handle_trap` 함수 또한 `kernel.c`에 작성한다. CSR에 저장된 예외에 대한 정보를 읽어와 `PANIC`으로 출력한다.
```c
void handle_trap(struct trap_frame* f) {
    uint32_t scause = READ_CSR(scause);
    uint32_t stval = READ_CSR(stval);
    uint32_t user_pc = READ_CSR(sepc);

    PANIC("unexpected trap scause=%x, stval=%x, sepc=%x\n", scause, stval, user_pc);
}
```
마지막으로 `kernel_main`함수에서 `kernel_entry`함수를 예외 핸들러로 등록해주고, 예외 상황을 발생시키는 명령을 호출한다.
```c
void kernel_main(void) {
    memset(__bss, 0, (size_t) __bss_end - (size_t) __bss);

    // CPU는 stcvec 레지스터에 저장된 주소를 예외 핸들러 주소로 사용한다.
    WRITE_CSR(stvec, (uint32_t) kernel_entry);

    const char* s = "\n\nhello world!\n";
    printf("console: %s", s);
    printf("date: %d %d %d\n", 2025, 1, 0);

    // unimp 명령 호출시 예외가 발생한다.
    __asm__ __volatile__("unimp");

    // 커널 패닉이 발생하므로 아래 코드는 실행되지 않는다.
    printf("%x\n", 0x1234abcd);

    for (;;) {
        __asm__ __volatile__("wfi");
    }
}
```

### 실행
`./run.sh`을 실행하면 아래와 같이 출력된다.
```
console: 

hello world!
date: 2025 1 0
PANIC: kernel.c:39 unexpected trap scause=00000002, stval=00000000, sepc=8020012e
```

출력된 정보를 보면 프로그램 카운터(`sepc`)가 `8020012e`로 되어있다. `llvm-objdump -d kernel.elf`를 사용해 프로그램에서 `8020012e` 주소의 명령을 살펴보면, 

```shell
$ llvm-objdump -d kernel.elf
kernel.elf:     file format elf32-littleriscv

Disassembly of section .text:

80200000 <boot>:
80200000: 37 05 22 80   lui     a0, 524832
80200004: 13 05 c5 4a   addi    a0, a0, 1196
...
8020012e: 00 00         unimp   
...
```

`unimp`이 호출된 지점임을 확인할 수 있다.

## 메모리 할당

### 링커 스크립트에 메모리 영역 설정

일반적인 OS는 부팅 시 하드웨어로부터 가용 메모리 크기를 받아와 사용한다. 여기서는 `kernel.ld` 파일에 메모리 영역을 설정하여 사용하겠다.

```linkerscript
. = ALIGN(4096);
__free_ram = .;
. += 64 * 1024 * 1024; /* 64MB */
__free_ram_end = .;
```

메모리는 페이지 단위로 할당하며, 1 페이지 당 4kb(=4096byte) 크기를 사용할 것이다. 따라서 메모리 시작 주소를 4096으로 정렬해주면 이후로 페이지 시작주소는 0x1000, 0x2000... 처럼 16진수로 보았을 때 보기 편하게 할당된다.

### 간단한 메모리 할당 알고리즘

일반적으로 메모리 해제가 필요한 메모리 할당을 위해서는 `buddy system`과 같은 알고리즘을 사용한다. 우선 다른 기능들을 개발하기 위해 메모리 해제가 불가능하지만 간단한 선형 할당 방식을 사용하고, 후에 보완하도록 하겠다.

`common.h`에 페이지 크기를, `kernel.c`에 메모리 할당 함수를 정의한다. 이후 `kernel_main` 함수에서 메모리 할당을 시도하고 할당받은 주소를 출력해보면 잘 동작함을 확인할 수 있다.

```c
// common.h
#define PAGE_SIZE 4096

//kernel.c
paddr_t alloc_pages(uint32_t n) {
    
    static paddr_t next_paddr = (paddr_t) __free_ram;
    // static 변수로 선언하였다. 초기값은 메모리 시작 주소이다.

    paddr_t curr_paddr = next_paddr;
    next_paddr += (n * PAGE_SIZE);
    // 메모리 할당시 요청한 페이지의 크기만큼 증가시킨다.

    if (next_paddr > (paddr_t) __free_ram_end) {
        // 메모리 용량을 넘어갈 경우 커널 패닉을 일으킨다.
        PANIC("out of memory: using %x, available: %x, tried to allocate %x.", \
        curr_paddr, (paddr_t)__free_ram_end, next_paddr);
    }

    memset((void*) curr_paddr, 0, n * PAGE_SIZE);
    // 할당된 메모리는 초기화시켜 제공한다.

    return curr_paddr;
}

...

void kernel_main(void) {
    ...
    paddr_t paddr0 = alloc_pages(2);
    paddr_t paddr1 = alloc_pages(1);
    printf("alloc pages test: paddr0 = %x\n", paddr0);
    printf("alloc pages test: paddr1 = %x\n", paddr1);
    
    paddr_t paddr2 = alloc_pages(64 * 1024);
    ...
}

```

```shell
$ ./run.sh
...
alloc pages test: paddr0 = 80221000 # 메모리 주소가 16진수 1000 단위로 할당된다.
alloc pages test: paddr1 = 80223000
PANIC: kernel.c:134 out of memory: using 80224000, available: 84221000, tried to allocate 90224000.
# 메모리 용량을 넘어서면 커널 패닉을 일으킨다.

$ llvm-nm kernel.elf | grep __free_ram
80221000 B __free_ram
84221000 B __free_ram_end
```

## 프로세스

### 프로세스 제어 블록(PCB)

프로세스 제어 블록(PCB)는 프로세스 ID(pid), 상태 등을 저장한다. `kernel.h`에 다음 구조체로 PCB를 정의한다.
```c
#define PROCS_MAX 8         // 프로세스 최대 개수

#define PROC_UNUSED 0       // 프로세스 상태
#define PROC_RUNNABLE 1

struct process {
    int pid;                // 프로세스 ID
    int state;              // 프로세스 상태 
    vaddr_t sp;             // 커널 스택 스택포인터
    uint8_t stack[8192];    // 커널 스택
};
```

프로세스는 커널 스택과 유저 스택을 갖는다. 유저 스택은 해당 프로세스가 사용하는 스택 영역으로 지역 변수, 함수 매개변수, 반환 값 등 일반적으로 스택이라 했을 때 저장하는 값이 들어있다. 커널 스택은 OS가 사용하는 스택 영역으로 커널 함수 내 변수, 시스템 호출 매개 변수, 커널 데이터 포인터, 인터럽트 시 레지스터 값 등을 저장한다. 

각 프로세스마다 커널 스택을 갖는 설계도 있지만 반대로 싱글 커널 스택이라 불리는 구조도 있다. 각 프로세스/스레드마다 커널 스택을 갖는 대신, CPU 하나에 커널 스택 하나씩을 갖는 구조이다. 

### 컨텍스트 스위칭
`kernel.c` 파일안의 `context_switch`함수로 컨텍스트 스위칭을 구현한다.
```c
 // 컴파일러의 함수 안의 인라인 어셈블리 외의 코드 생성을 막는다. 
__attribute__((naked))     
// 현재 프로세스의 커널 스택 포인터와 실행할 프로세스의 커널 스택 포인터를 인자로 받는다.
void switch_context(uint32_t *prev_sp, uint32_t *next_sp) {
    __asm__ __volatile__ (
            // 현재 커널 스택에 레지스터 값을 저장한다. 이때, caller-saved를 제외한 callee-saved 레지스터들만을 저장한다. caller-saved 레지스터는 함수를 호출한 쪽에서 저장할 책임이 있지만, callee-saved 레지스터는 호출된 함수가 책임이 있다. 호출된 함수가 리턴하기 전에 해당 레지스터들의 값을 호출되기 전과 같은 값으로 만들어야 한다.
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

            // 현재 프로세스의 커널 스택 포인터 값을 업데이트 해준다.
            "sw sp, (a0)\n"
            // 실행할 프로세스의 커널 스택 포인터 값을 가져온다.
            "lw sp, (a1)\n"
            
            // 실행한 프로세스의 커널 스택에 저장되어 있던 레지스터 값들을 불러온다.
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
```

다음으로, 프로세스를 생성 및 초기화하는 `create_process` 함수를 `kernel.c`에 작성한다. `create_process` 함수는 `procs` 배열에서 비어있는 프로세스 슬롯을 찾고 pid와 entry point를 설정하여 PCB 포인터를 반환한다.

```c
// entry_point는 함수 주소 값이다.
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
    // 커널 스택에 레지스터 초기값들을 넣는다.
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
    *--sp = entry_point; // ra - 리턴주소를 저장하는 레지스터이다. switch_context 함수의 마지막이 'ret' 명령어이므로, 해당 주소에 있는 함수를 실행하게 된다.

    proc->pid = i;
    proc->state = PROC_RUNNABLE;
    proc->sp = (uint32_t) sp;

    return proc;
}
```

이제 프로세스 A와 B를 만들고 컨텍스트 스위칭을 테스트해보자. `kernel.c`에 아래와 같은 코드를 작성하고 실행한다.

```c
// 출력이 너무 빠르게 일어나는 것을 방지하기 위한 함수이다.
void delay() {
    for (int i = 0; i < 30000000; i++) {
        // 컴파일러 최적화로 인해 반복문이 생략되는 것을 막기 위함.
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
        switch_context(&proc_a->sp, &proc_b->sp);
    }
}

void proc_b_entry() {
    printf("start process b\n");
    while(1) {
        printf("pid %d B\n", proc_b->pid);
        delay();
        switch_context(&proc_b->sp, &proc_a->sp);
    }
}

...

void kernel_main(void) {
    memset(__bss, 0, (size_t) __bss_end - (size_t) __bss);

    WRITE_CSR(stvec, (uint32_t) kernel_entry);

    proc_a = create_process((uint32_t) proc_a_entry);
    proc_b = create_process((uint32_t) proc_b_entry);

    proc_a_entry();

    PANIC("never executed.");
}
```

```shell
$ ./run.sh

...
# 아래와 같은 출력이 계속해서 이어진다면 성공이다.
start process a
pid 0 A
start process b
pid 1 B
pid 0 A
pid 1 B
pid 0 A
pid 1 B
...
```

### 스케줄러

매번 프로세스에서 전환할 프로세스를 일일히 지정해주어야 한다면 복잡하고 버그 발생 확률이 높아진다. 따라서 다음에 실행할 프로세스를 결정해주는 커널 프로그램인 스케줄러가 필요하다. `kernel.c` 파일에 작성한 다음 `yield` 함수로 간단한 비선점형 스케줄러를 구현할 수 있다.

```c
// 현재 실행되고 있는 프로세스
struct process *current_proc;
// 유휴 프로세스 - 아무런 프로세스도 실행되고 있지 않을 때 해당 프로세스를 실행한다.
struct process *idle_proc;

// yield 함수를 호출하면 실행 가능한 다른 프로세스를 찾아 실행한다.
void yield() {
    struct process *next_proc = idle_proc;
    for (int i = 1; i <= PROCS_MAX; i++) {
        // 현재 실행 중인 프로세스 다음 프로세스부터 한 바퀴를 쭉 돌면서 실행 가능한 프로세스를 찾는다.
        struct process *proc = &procs[(current_proc->pid + i) % PROCS_MAX];
        if (proc->state == PROC_RUNNABLE && proc->pid > 0) {
            next_proc = proc;
            break;
        }
    }

    // 다른 실행가능한 프로세스가 없다면 컨텍스트 스위칭을 하지 않고 실행중이던 프로세스를 계속 실행한다.
    if (next_proc == current_proc) return;

    // sscratch 레지스터에 커널스택 시작 주소를 저장한다. 이유는 아래에서 설명.
    WRITE_CSR(sscratch, (uint32_t) &next_proc->stack[sizeof(next_proc->stack)]);

    // 현재 실행중인 프로세스 정보를 바꾸고 다음 프로세스를 실행한다.
    struct process* prev_proc = current_proc;
    current_proc = next_proc;
    switch_context(&prev_proc->sp, &next_proc->sp);
}
```

`kernel_main`, `proc_a_entry`, `proc_b_entry`도 다음과 같이 변경한다.

```c
void proc_a_entry() {
    printf("start process a\n");
    while(1) {
        printf("pid %d A\n", proc_a->pid);
        delay();
        yield(); // 직접 다음 프로세스를 진행하는 대신 yield 호출
    }
} // proc_b_entry도 똑같이 전환

void kernel_main(void) {
    memset(__bss, 0, (size_t) __bss_end - (size_t) __bss);

    WRITE_CSR(stvec, (uint32_t) kernel_entry);

    // idle 프로세스 생성 및 초기화
    idle_proc = create_process((uint32_t) NULL);
    idle_proc->pid = -1;
    current_proc = idle_proc;

    proc_a = create_process((uint32_t) proc_a_entry);
    proc_b = create_process((uint32_t) proc_b_entry);

    // idle 프로세스에서 다른 프로세스로 전환
    yield();

    PANIC("switched to idle process");
}
```

실행하면 이전과 똑같이 프로세스 간에 컨텍스트 스위칭이 잘 동작한다.

### 예외 핸들러 수정

예외 핸들러를 다음과 같이 수정해야한다. 이전에는 스택에 바로 레지스터 값을 저장하고 불러왔지만, 이제는 `yield` 함수 호출 시 `sscratch` 레지스터에 저장해둔 커널 스택 주소값을 가져와 해당 커널 스택에 레지스터를 저장하고 불러온다.

```c
__attribute__((naked))
__attribute__((aligned(4)))
void kernel_entry(void) {
    __asm__ __volatile__(
            "csrrw sp, sscratch, sp\n"  // 현재 스택포인터 값을 sscratch에 저장하고 sscratch에 저장된 커널스택 스택포인터 값을 가져온다.
            "addi sp, sp, -4 * 31\n"    // 현재 스택 포인터 -= 4 * 31 (32바이트 크기의 스택 확보)

            "sw ra,  4 * 0(sp)\n"       // 스택에 현재 레지스터 값 전부 저장
            "sw gp,  4 * 1(sp)\n"
            // ...
            "sw s11, 4 * 29(sp)\n"

            "csrr a0, sscratch\n"       // 저장해둔 스택 포인터 값을 다시 불러와서 스택에 저장
            "sw a0, 4 * 30(sp)\n"

            "addi a0, sp, 4 * 31\n"     // 커널 스택 값을 다시 복원해서 sscratch에 저장
            "csrw sscratch, a0\n"

            "mv a0, sp\n"               // a0 레지스터에 현재 스택 포인터값을 넣어 handle_trap 호출 시 인자로 넘겨준다.
            "call handle_trap\n"        // handle_trap 함수 호출


            "lw ra,  4 * 0(sp)\n"       // 스택에 넣어두었던 값을 다시 레지스터로 가져온다.
            "lw gp,  4 * 1(sp)\n"
            //...
            "lw sp, 4 * 30(sp)\n"

            "sret\n"                  // kernel_entry가 호출되었던 지점으로 복귀
            );
}
```

이와 같이 구현한 이유는 실행 중인 프로세스가 유저 모드에서 오류가 발생했을 때 무한 루프를 막기 위해서이다. 유저 모드에서 스택 오버플로우 등으로 스택 포인터 값이 유효하지 않은 주소를 갖게되는 상황을 가정해보자. 이 때 스택 포인터 레지스터를 그대로 사용하게 되면 유효하지 않은 주소에 접근하게 되어 또다시 인터럽트가 발생하고, 에러 핸들러가 호출되어 무한 루프에 빠지게 된다. 유저 모드에서는 커널 스택이 아닌 유저 스택을 사용하고 있기 때문에 유저 모드에서 오류가 나더라도 커널 스택에는 영향이 없다. 따라서 유효한 주소임이 보장된 커널 스택 시작 주소 값에 레지스터를 저장하고, 불러오는 것이다.

### 페이지 테이블

운영체제는 다양한 이유로 물리 주소를 직접 사용하지 않고 논리 주소(가상 주소)를 사용한다. 

첫번째로 보안적 이점이 있다. 물리 주소를 직접 사용하게 되면 프로세스가 다른 프로세스나 심지어는 커널이 사용중인 메모리 주소에 접근할 수 있다.

두번째로 메모리 용량에서 이점이 있다. 논리 주소를 이용하면 가용 물리 메모리 용량보다 많은 메모리를 사용할 수 있다.

이때 논리 주소를 물리 주소로 변환하기 위해 사용되는 것이 페이지 테이블이다. 페이지 테이블은 아래와 같이 구성된다. 이떄 매핑된 인덱스들은 메모리 할당 단위인 페이지(일반적으로 4KB)의 인덱스이다. 페이지 테이블의 각 행을 페이지 엔트리라고 부른다.

| 논리 페이지 인덱스 | 물리 페이지 인덱스 |
|------------|------------|
| 0          | 2          |
| 1          | 8          |
| 2          | 10         |
...

만약 프로세스가 0x00001024 주소에 접근한다고 가정하자. 32bit 중 4KB(2^12)에 해당하는 부분인 024는 offset값으로, 그 앞의 00001은 페이지 인텍스로 사용된다. 페이지 테이블을 참조하면 1번 페이지는 물리 8번 페이지에 매핑되어 있으므로, 0x00008024 주소를 참조하게 된다.

이렇게 페이지 테이블을 1개만 사용할 경우 페이지 테이블의 용량 문제가 발생한다. 페이지는 일반적으로 4KB(2^12) 용량을 갖는다. 32비트 아키텍처에서 사용할 수 있는 주소는 4GB(2^32)가지이므로, 2^20(1M)개의 페이지 엔트리가 필요하다. 페이지 엔트리 1개 당 4Byte라고 계산하면 4GB의 메모리를 사용하기 위해 4MB 크기의 페이지 테이블이 필요하다.

페이지 테이블의 용량을 줄이기 위한 방법 중 하나로 다단계 페이지 테이블을 사용한다. 다단계 페이지 테이블은 접근 시간을 포기하는 대신 사용하지 않는 페이지에 대한 테이블을 제거하여 용량을 줄일 수 있다. 

2개의 페이지 테이블을 사용하는 2단계 페이지 테이블에서 각 테이블의 페이지 엔트리 개수는 1KB(2^10)개로, 2단계를 거쳐 1KB * 1KB = 1M (2^10 * 2^10 = 2^20)개의 엔트리를 표현할 수 있다. 또한 엔트리 1개당 4KB이므로 4KB의 용량을 가져 1개의 페이지에 1개의 테이블을 저장할 수 있다. 

마찬가지로 00001024 주소에 접근할 경우, 주소를 아래와 같이 사용한다. [RISC-V Sv-32 Virtual Address Breakdown](https://riscv-sv32-virtual-address.vercel.app/)을 참고하여 주소가 어떻게 쪼개지는지 확인할 수 있다.

|    | 1단계 테이블 인텍스  | 2단계 테이블 인덱스  | offset         |
|----|--------------|--------------|----------------|
| 0b | 0000 0000 00 | 00 0000 0001 | 0000 0010 0100 |
| 0x | ___0 ___0 __ | _0 ___0 ___1 | ___0 ___2 ___4 |

> Tip
> 주소의 변화를 관찰해보면 다음 2가지 사실을 알 수 있다.
> 1. 0-11번째 비트 변경시 테이블 인덱스는 변하지 않는다. (같은 물리 페이지를 참조한다.)
> 2. 12-21번째 비트 변경시 1단계 테이블 인덱스는 변하지 않는다. (같은 2단계 테이블을 참조한다.)
> 공간 지역성을 활용하는 구조임을 알 수 있다. 논리 주소를 물리 주소로 변환할 때 참조 시간을 줄이기 위해 TLB(Transition Lookaside Buffer)을 사용해 캐싱하는데, 이와 같은 구조를 사용해 캐시 Hit rate을 높일 수 있다.

따라서 1단계 인덱스는 0, 2단계 인덱스는 1, offset은 24가 된다. 페이지 테이블이 아래와 같이 구성되어있다고 하면,
1. 1단계 테이블의 0번째 엔트리를 참조하여 2번째 2단계 테이블을 확인한다.
2. 2번째 2단계 테이블의 1번째 엔트리를 참조하여 8번째 물리 페이지를 확인한다.
3. 8번째 물리 페이지에서 offset이 24인, 즉 0x00008024 주소를 최종적으로 참조한다.

1단계 테이블:

| 1단계 테이블 인덱스 | 2단계 테이블 인덱스 |
|-------------|-------------|
| 0           | 2           |
| 1           | 8           |
| 2           | 10          |
...

2번째 2단계 테이블:

| 논리 페이지 인덱스 | 물리 페이지 인덱스 |
|------------|------------|
| 0          | 5          |
| 1          | 8          |
| 2          | 9          |
...

만약 프로세스가 4GB의 메모리를 전부 사용한다면 1KB(2^12)개의 2단계 테이블과 1개의 1단계 테이블이 필요하므로 용량은 4MB + 4KB가 된다. 하지만 사용하지 않는 메모리가 있다면 그 부분에 해당하는 2단계 테이블을 생성하지 않으므로 그만큼의 메모리를 줄일 수 있다. 예를 들어 메모리를 2GB만 사용한다면 0.5KB개의 2단계 테이블과 1개의 1단계 테이블만 있으면 되므로 페이지 테이블이 차지하는 용량이 절반가량 줄어든다. 

1단계 테이블은 항상 4KB의 용량을 차지하고, 모든 2단계 테이블에 대한 엔트리를 갖고 있다. 단, 존재하지 않는 2단계 테이블에 대한 엔트리의 경우 null값을 저장한다. 이렇게 구성하는 이유는 페이지 테이블의 엔트리에 접근할 때 배열을 사용하여 시간 복잡도를 O(1)로 만들기 위함이다.

`kernel.c`에 작성한 아래 `map_page`함수는 1단계 테이블, 논리주소 및 물리주소, 권한 설정을 위한 플래그를 받아 페이지 테이블을 생성하고 주소를 매핑해주는 함수이다. 사용된 매크로는 `kernel.h`에 있다.

```c
// kernel.h
// 이후 페이지 테이블을 활성화하기 위해 레지스터에 값을 넣을 때 설정해야하는 비트이다. 해당 비트가 1로 설정되어 있어야 Sv32 모드로 페이징을 할 수 있다.
#define SATP_SV32 (1u << 31)
// 아래 매크로는 페이지 권한 설정을 위한 비트이다.
#define PAGE_V    (1 << 0)
#define PAGE_R    (1 << 1)
#define PAGE_W    (1 << 2)
#define PAGE_X    (1 << 3)
#define PAGE_U    (1 << 4)

// kernel.c
void map_page(uint32_t* table1, vaddr_t vaddr, paddr_t paddr, uint32_t flags) {
    // 논리 주소 또는 물리 주소가 페이지의 시작점을 가르키고 있지 않을 경우 패닉을 일으킨다.
    if (!is_aligned(vaddr, PAGE_SIZE))
        PANIC("unaligned vaddr %x\n", vaddr);
    
    if (!is_aligned(paddr, PAGE_SIZE))
        PANIC("unaligned paddr %x\n", paddr);
    
    // 1단계 테이블에서 조회할 엔트리의 인덱스를 계산한다.
    uint32_t vpn1 = (vaddr >> 22) & 0x3FF;
    // 해당 엔트리가 가르키고 있는 2단계 테이블이 없을 경우, 2단계 테이블을 새로 만든다.
    // PAGE_V는 Validation Bit로, 주소 0과 NULL을 구분하기 위해 사용한다.
    if ((table1[vpn1] & PAGE_V) == 0) {
        paddr_t table2_page_addr = alloc_pages(1);
        // 페이지 크기로 나누는 이유는 실제 페이지 주소가 아니라 몇번째 페이지인지를 저장해야하기 때문이다.
        // 이후 Validation Bit 설정을 위해 시프트하고 엔트리에 해당 값을 저장한다.
        table1[vpn1] = ((table2_page_addr / PAGE_SIZE) << 10) | PAGE_V;
    }

    // 2단계 테이블 주소를 계산한다.
    paddr_t* table2 = (paddr_t*) ((table1[vpn1] >> 10) * PAGE_SIZE);
    // 2단계 테이블에서 조회할 엔트리의 인덱스를 계산한다.
    uint32_t vpn2 = (vaddr >> 12) & 0x3FF;
    // 마찬가지로 물리 주소를 페이지 크기로 나누고 플래그를 넣어준다.
    table2[vpn2] = ((paddr / PAGE_SIZE) << 10) | PAGE_V | flags;
}
```

다음으로는 프로세스 생성시 커널 메모리 영역을 페이지 테이블에 넣어주어야 한다. 여기서는 페이징을 활성화시킨 후에도 코드가 똑같이 동작할 수 있도록 커널 메모리 영역의 논리 주소와 물리 주소를 일치시킨다. 커널 메모리 영역은 메모리 시작점부터 `__free_ram_end`까지이다. 메모리 시작점을 가져오기 위해 링커 스크립트에 `__kernel_base` 변수를 선언해주자.

```linkerscript
ENTRY(boot)

SECTIONS {
    . = 0x80200000;
    __kernel_base = .;
     /* 생략 */
```

이후 `kernel.h`에 정의된 PCB 구조체에 `page_table` 속성을 추가하고 `kernel.c`에 정의된 `create_process` 함수를 아래와 같이 수정해주자.
```c
// 링커 스크립트에서 __kernel_base를 가져온다.
extern char __kernel_base[];

struct process* create_process(uint32_t entry_point) {
    // 생략
    
    // 페이지 테이블을 저장할 페이지를 생성한다.
    paddr_t page_table = alloc_pages(1);
    for (
        // 커널 메모리 시작점 ~ 끝점까지
        paddr_t addr = (paddr_t)__kernel_base;
        addr < (paddr_t)__free_ram_end;
        addr += PAGE_SIZE
    ) {
        // 페이지를 생성하고 매핑한다.
        paddr_t page = alloc_pages(1);
        map_page(
            (uint32_t *)page_table,
            addr,
            addr,
            PAGE_R | PAGE_W | PAGE_X
        );
    }

    proc->pid = i;
    proc->state = PROC_RUNNABLE;
    proc->sp = (uint32_t) sp;
    
    // PCB에 페이지 테이블 주소값을 저장한다.
    proc->page_table = page_table;
    
    return proc;
}
```

마지막으로 컨텍스트 스위칭 시 페이지 테이블을 설정해준다.

```c
void yield() {
    // 생략

    __asm__ __volatile__ ("sfence.vma");
    WRITE_CSR(satp, (SATP_SV32 | ((uint32_t) next_proc->page_table / PAGE_SIZE)));
    __asm__ __volatile__ ("sfence.vma");
    
    WRITE_CSR(sscratch, (uint32_t) &next_proc->stack[sizeof(next_proc->stack)]);

    struct process* prev_proc = current_proc;
    current_proc = next_proc;
    switch_context(&prev_proc->sp, &next_proc->sp);
}
```
`sfence.vma`는 supervisor fence virtual memory adress라는 뜻으로, CPU의 메모리 접근을 제어하는 MMU의 캐시를 지워버린다. 캐시를 지우는 이유는 캐싱 시 사용될 주소를 미리 예측할 수 있기 때문이다. 만약 캐시를 지우지 않았다면 캐시 미스가 일어나고 페이지 테이블을 참조하여 물리 주소를 읽어오는, 시간이 오래걸리는 작업을 해야한다. 캐시를 지움으로써 MMU가 미리 주소를 캐싱하게 되고, 결과적으로 성능을 향상시킬 수 있다. 

참고: https://blog.stephenmarz.com/2021/02/01/wrong-about-sfence/

실행해보면 이전과 똑같은 출력이 나온다. 페이지 테이블이 정상적으로 설정되었는지 확인하려면 QEMU의 디버깅 기능을 이용하면 된다. `ctrl+c, a`를 눌러 디버깅 모드에 진입하고, `info mem`을 입력하면 아래와 같은 페이지 테이블을 확인할 수 있다.

```shell
(qemu) info mem
vaddr    paddr            size     attr
-------- ---------------- -------- -------
80200000 0000000080200000 00001000 rwx--ad
80201000 0000000080201000 00001000 rwx----
80202000 0000000080202000 00001000 rwx--a-
80203000 0000000080203000 00001000 rwx----
80204000 0000000080204000 00001000 rwx--ad
80205000 0000000080205000 00001000 rwx----
80206000 0000000080206000 00001000 rwx--ad
80207000 0000000080207000 00009000 rwx----
...
84000000 0000000084000000 00231000 rwx----
```

커널 메모리 영역 시작점인 `0x80200000`부터 매핑이 시작되어 논리주소와 물리주소가 일치하게 매핑되어있음을 확인할 수 있다.

