#include <stddef.h>
#include <stdint.h>

uint32_t pseudo_rand();
void *memcpy(void *restrict dest, const void *restrict src, size_t n);
void *memset(void *s, int c, size_t n);
void *memmove(void *dest, const void *src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);
long syscall1(long n, long a1);
long syscall2(long n, long a1, long a2);
uint64_t malloc(uint64_t n);
int fread(int fd, char *buf, int n);
int fopen(char *filename);
int fclose(int fd);
int fseek(int fd, int offset);
void reverse(char* str, int length);
void print(char* str);
void int_to_str(uint64_t num, char* str);
void exec(char* fname, char* arg);
void read_input(char* buffer);
void exit(int code);
void print_h(char* address, int n);
int strcmp(const char* s1, const char* s2);