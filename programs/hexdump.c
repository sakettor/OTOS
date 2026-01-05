#include <stdint.h>
#include "lib.h"

void main(char* filename) {
    int file = fopen(filename);
    if (file == -1) {
        print("hexdump: file not found: ");
        print(filename);
        print("\n");
        fclose(file);
        exit(1);
    }
    char *buffer = (char*)malloc(510);
    memset(buffer, 0, 4096);
    int bytes = fread(file, buffer, 510);
    if (bytes > 0) {
        print_h(buffer, bytes);
    }
    print("\n");
    fclose(file);
    exit(0);
}