# Hello world!

## ecall

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

## printf

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
