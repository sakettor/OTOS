.intel_syntax noprefix
.section .text

.global gdt_reload
.global usermode_entry

gdt_reload:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    push 0x08
    lea rax, [rip + 1f]
    push rax
    lretq
1:
    mov ax, 0x28
    ltr ax
    ret
usermode_entry:
    push 0x23
    push rsi
    push 0x202
    push 0x1B
    push rdi
    mov ax, 0x23
    mov ds, ax
    mov es, ax
    iretq 