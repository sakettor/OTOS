#include <stdint.h>

uint32_t seed = 12345;
uint32_t pseudo_rand() {
    seed = seed * 1103515245 + 12345;
    return (seed / 65536) % 32768;
}

long syscall1(long n, long a1) {
    long ret;
    __asm__ volatile (
        "int $0x80" 
        : "=a"(ret)
        : "a"(n), "b"(a1) 
        : "rcx", "r11", "memory"
    );
    return ret;
}

void print(char* str) {
    syscall1(1, (long)str);
}

void exec(char* fname, char* arg) {
    asm volatile (
        "int $0x80" 
        : 
        : "a"(5), "b"(fname), "d"(arg) 
        : "memory"
    );
}

void read_input(char* buffer) {
    syscall1(2, (long)buffer);
}

void exit(int code) {
    syscall1(4, code);
}

void print_h(char* address, int n) {
    for (int i = 0; i < n; i++) {
        if (i % 30 == 0) {
            print("\n");
        }
        char byte_str[4];
        uint8_t byte = address[i];
        const char hex_chars[] = "0123456789ABCDEF";
        byte_str[0] = hex_chars[(byte >> 4) & 0x0F];
        byte_str[1] = hex_chars[byte & 0x0F];
        byte_str[2] = ' ';
        byte_str[3] = '\0';
        print(byte_str);
    }
}

int strcmp(const char* s1, const char* s2) {
    while(*s1 && (*s1 == *s2))
    {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}