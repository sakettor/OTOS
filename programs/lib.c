#include <stdint.h>
#include <stddef.h>

uint32_t seed = 12345;
uint32_t pseudo_rand() {
    seed = seed * 1103515245 + 12345;
    return (seed / 65536) % 32768;
}

void *memcpy(void *restrict dest, const void *restrict src, size_t n) {
    uint8_t *restrict pdest = (uint8_t *restrict)dest;
    const uint8_t *restrict psrc = (const uint8_t *restrict)src;

    for (size_t i = 0; i < n; i++) {
        pdest[i] = psrc[i];
    }

    return dest;
}

void *memset(void *s, int c, size_t n) {
    uint8_t *p = (uint8_t *)s;

    for (size_t i = 0; i < n; i++) {
        p[i] = (uint8_t)c;
    }

    return s;
}

void *memmove(void *dest, const void *src, size_t n) {
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;

    if (src > dest) {
        for (size_t i = 0; i < n; i++) {
            pdest[i] = psrc[i];
        }
    } else if (src < dest) {
        for (size_t i = n; i > 0; i--) {
            pdest[i-1] = psrc[i-1];
        }
    }

    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    const uint8_t *p1 = (const uint8_t *)s1;
    const uint8_t *p2 = (const uint8_t *)s2;

    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] < p2[i] ? -1 : 1;
        }
    }

    return 0;
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

long syscall2(long n, long a1, long a2) {
    long ret;
    __asm__ volatile (
        "int $0x80" 
        : "=a"(ret)
        : "a"(n), "b"(a1), "d"(a2)
        : "rcx", "r11", "memory"
    );
    return ret;
}

uint64_t malloc(uint64_t n) {
    uint64_t ret = syscall1(8, n);
    return ret;
}

int fread(int fd, char *buf, int n) {
    long ret;
    __asm__ volatile (
        "int $0x80"
        : "=a"(ret)
        : "a"(6),
          "b"((long)fd),
          "d"((long)buf),
          "c"((long)n)
        : "memory"
    );
    return (int)ret;
}

int fopen(char *filename) {
    long ret = syscall1(9, (long)filename);
    return ret;
}

int fclose(int fd) {
    long ret = syscall1(10, (long)fd);
    return ret;
}

int fseek(int fd, int offset) {
    long ret = syscall2(11, (long)fd, offset);
    return ret;
}

void reverse(char* str, int length) {
    int start = 0;
    int end = length - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

void print(char* str) {
    syscall1(1, (long)str);
}

void int_to_str(uint64_t num, char* str) {
    int i = 0;
    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return;
    }
    while (num != 0) {
        int rem = num % 10;
        str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        num = num / 10;
    }
    str[i] = '\0';
    reverse(str, i);
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