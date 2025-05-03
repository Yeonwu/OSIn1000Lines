# RISC-V 기초

## 어셈블리 기초

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

### 메모리 접근

레지스터는 빠르지만 개수가 제한되어있다. 따라서 다음과 같은 흐름으로 데이터를 다룬다.

메인 메모리 > 레지스터 > (연산) > 레지스터 > 메인 메모리

이때 레지스터에서 메모리로, 메모리에서 레지스터로 데이터를 옮기는데 사용하는 명령어가 lw(load word, 메모리 > 레지스터), sw(save word, 레지스터 > 메모리)이다.

```asm
// 괄호 안의 값은 메모리 주소이다.

lw a0 (a1) // a1 주소에 저장된 word(32-bit 아키텍처이므로 32bit 크기)를 a0 레지스터에 저장한다. C로 치면 a0 = *a1;
sw a0 (a1) // a0 레지스터에 저장된 word를 a1 주소에 저장한다. C로 치면 *a1 = *a0;
```

### 브랜치 명령어

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

### 스택

FIFO구조의 메모리로, 아래방향으로 확장된다. 함수 호출, 로컬 변수 등에 사용된다. `sp` 레지스터에 스택의 top 주소를 저장한다.

Push, Pop 연산은 다음과 같이 구현할 수 있다. 
```asm
addi sp, sp, -4 // sp = sp - 4
sw a0, sp       // *sp = a0

lw sp, a0       // a0 = *sp
addi sp, sp, 4  // sp = sp + 4
```

### CPU 모드 & Privileged Instructions

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

## 인라인 어셈블리

C코드에 어셈블리를 삽입할 수 있다. 인라인 어셈블리를 사용하면 어셈블리 파일을 따로 작성하는 것에 비해 다음 2가지 장점이 있다.
- C에서 사용하고 있는 변수를 바로 어셈블리로 가져올 수 있고, 어셈블리에서 C언어 변수에 값을 할당할 수 있다. 
- 레지스터 할당을 컴파일러가 알아서 해주기 때문에, 어떤 레지스터를 사용중인지 일일히 기억하며 코드를 짤 필요가 없다.

### 문법

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

### 예시

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
