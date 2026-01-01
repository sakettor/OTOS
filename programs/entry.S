[bits 64]
extern main
global _start

_start:
    call main
    mov rax, 4
    int 0x80