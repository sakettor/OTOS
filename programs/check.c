#include <stdint.h>
#include "lib.h"

int nice = 0;

void main(char* filename) {
    print("OTOS system check\n");

    if (filename && filename[0] != '\0') {
        print("argument check ok: ");
        print(filename);
        print("\n");
        nice++;
    } else {
        print("argument check fail\n");
    }

    print("memory operations check ");
    char* buf1 = (char*)malloc(1024);
    char* buf2 = (char*)malloc(1024);

    if (buf1 && buf2 && buf1 != buf2) {
        memset(buf1, 0xAA, 1024);
        memcpy(buf2, buf1, 1024);
        if (memcmp(buf1, buf2, 1024) == 0) {
            print("ok\n");
            nice++;
        } else {
            print("fail, data corrupted\n");
        }
    } else {
        print("fail\n");
    }

    print("tarfs open, read and seek operations check\n");
    
    int fd = fopen(filename);
    if (fd >= 0) {
        print("open operation ok\n");
        nice++;
        memset(buf1, 0, 1024);
        int bytes = fread(fd, buf1, 10);
        
        if (bytes > 0) {
            print("read operation ok (");
            buf1[bytes > 10 ? 10 : bytes] = '\0';
            print(buf1);
            print(")\n");
            nice++;
            
            fseek(fd, 5);
            memset(buf2, 0, 1024);
            int bytes2 = fread(fd, buf2, 10);
            
            if (bytes2 > 0 && memcmp(buf1 + 5, buf2, 2) != 0) {
                 print("seek operation ok\n");
                 nice++;
            } else {
                 print("seek operation fail\n");
            }
        } else {
            print("read operation fail, read 0 bytes\n");
        }
        fclose(fd);
    } else {
        print("open operation fail (it the file truly on the disk?)\n");
    }

    print("math operations check\n");
    char num_buf[32];
    uint64_t math_check = (12345 * 2) + 6789; // 31479
    int_to_str(math_check, num_buf);
    print("result: ");
    print(num_buf);
    if (math_check == 31479) {
        print(" ok\n");
        nice++;
    } else {
        print(" fail\n");
    }

    if (nice == 6) {
        print("\n\nOTOS system check success\n");
    } else {
        print("\n\nOTOS system check fail (");
        char* nice_str = (char*)malloc(1);
        int_to_str(nice, nice_str);
        print(nice_str);
        print("/6)\n");
    }

    print("exiting\n");

    exit(0);
}