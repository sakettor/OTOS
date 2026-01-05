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
    mov rsp, rsi
    mov rsi, rsp
    mov rbx, rcx
    push 0x23
    push rsi
    push 0x202
    push 0x1B
    push rdi
    mov rdi, rdx
    xor rax, rax
    xor rcx, rcx
    xor rbp, rbp
    xor r8, r8
    xor r9, r9
    xor r10, r10
    xor r11, r11
    xor r12, r12
    xor r13, r13
    xor r14, r14
    xor r15, r15
    mov ax, 0x23
    mov ds, ax
    mov es, ax
    xor ax, ax
    mov fs, ax
    mov gs, ax

    iretq