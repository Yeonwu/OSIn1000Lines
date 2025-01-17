# OSIn1000Lines

참고: https://github.com/nuta/operating-system-in-1000-lines

## Getting Started

### Environment

- OS: Ubuntu 23.10
- IDE: CLion

### Installation
가상의 CPU를 애뮬레이트하기 위해 QEMU를 사용한다.

QEMU 설치 중 아래와 같은 오류가 발생하였다.

```terminal
$sudo apt update

E: The repository 'http://ports.ubuntu.com/ubuntu-ports mantic Release' no longer has a Release file.
N: Updating from such a repository can't be done securely, and is therefore disabled by default.
```

Ubuntu 23.10 EOL일자(2024.7.11)가 도래함으로 인해 지원이 종료되어 Repository에 접근이 막혔다. 정석적인 해결방법은 Ubuntu 버전을 업그레이드 하는 것이지만, 시간이 너무 오래걸리고 귀찮으므로 보안 취약성을 무릅쓰고 구버전 Repository를 그냥 사용하기로 결정했다.

`/etc/apt/source.list`의 원래 내용을 주석처리하고 아래 내용으로 변경하였다.
```terminal
deb http://old-releases.ubuntu.com/ubuntu/ ㅡmantic main restricted universe multiverse
deb http://old-releases.ubuntu.com/ubuntu/ ㅡmantic-updates main restricted universe multiverse
deb http://old-releases.ubuntu.com/ubuntu/ ㅡmantic-security main restricted universe multiverse

```
참고: https://help.ubuntu.com/community/EOLUpgrades


이후 정상적으로 QEMU 설치를 마쳤다.
```terminal
$sudo apt install qemu-system-riscv32
$curl -LO https://github.com/qemu/qemu/raw/v8.0.4/pc-bios/opensbi-riscv32-generic-fw_dynamic.bin

$qemu-system-riscv32 --version

QEMU emulator version 8.0.4 (Debian 1:8.0.4+dfsg-1ubuntu3.23.10.5)
Copyright (c) 2003-2022 Fabrice Bellard and the QEMU Project developers
```

OpenSBI는 BIOS랑 비슷한 거라고 생각하면 된다.
