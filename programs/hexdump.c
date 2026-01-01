#include <stdint.h>
#include "lib.h"

void main() {
    char* filename;
    asm ("mov %%rdx, %0;" : "=m"(filename));
    
    uint64_t var_rax;
    
    asm volatile (
        "mov %1, %%rbx;"
        "mov $6, %%rax;"
        "int $0x80;"
        : "=a"(var_rax)
        : "r"(filename)
        : "rbx", "memory"
    );

    if (var_rax != 0) {
        print_h((char *)var_rax, 1020);
        print("\n");
        exit(0);
    }
    exit(1);
}