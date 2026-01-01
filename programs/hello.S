.intel_syntax noprefix
.section .text
.global hello_sim

_start:
    mov rax, 1
    lea rbx, [rip + welcoming]
    int 0x80
    jmp hello_sim

strcmp:
.loop:
    mov al, [rdi]
    mov bl, [rsi]
    cmp al, bl
    jne .not_equal
    test al, al
    jz .equal
    inc rdi
    inc rsi
    jmp .loop

.not_equal:
    mov rax, 1
    ret
.equal:
    xor rax, rax
    ret

hello_sim:
    mov rax, 2
    lea rbx, [rip + input]
    int 0x80
    mov rdi, rbx
    lea rsi, [rip + hello]
    call strcmp
    cmp rax, 0
    je .true
    mov rax, 1
    lea rbx, [rip + wrong]
    int 0x80
    int 8
    jmp hello_sim
.true:
    mov rax, 1
    lea rbx, [rip + hello_back]
    int 0x80
    mov rax, 4
    mov rbx, 0
    int 0x80

    jmp hello_sim

.section .data
    hello:      .asciz "hello"
    hello_back: .asciz "Hi! how are you?\n"
    wrong: .asciz "Wrong, go learn etiquette you fatass bitch\n"
    welcoming: .asciz "Welcome to hello simulator!\n"
    exit: .asciz "exit"
    input:      .zero 500
    