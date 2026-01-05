#include <stdint.h>
#include "lib.h"

void main(char* filename) {
    int file = fopen(filename);
    if (file == -1) {
        print("type: file not found: ");
        print(filename);
        print("\n");
        fclose(file);
        exit(1);
    }
    char *buffer = (char*)malloc(4096);
    memset(buffer, 0, 4096);
    int bytes = fread(file, buffer, 4096);
    if (bytes > 0) {
        print(buffer);
    }
    print("\n");
    fclose(file);
    exit(0);
}