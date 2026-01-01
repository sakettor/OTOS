#include <stdint.h>
#include <stddef.h>
#include "lib.h"

#define OTVERSION "Version 0.2"

// argv stores pointers to the start of each word
char* argv[20]; 
// argc counts how many words we found
int argc = 0; 
static char arg_backup[64];

void parse_command(char* input) {
    argc = 0;
    int i = 0;

    while (input[i] != '\0') {
        while (input[i] == ' ') {
            input[i] = '\0'; 
            i++;
        }
        if (input[i] == '\0') {
            break;
        }
        if (argc < 20) {
            argv[argc] = &input[i];
            argc++;
        }
        while (input[i] != ' ' && input[i] != '\0') {
            i++;
        }
    }
}

void main() {
    char cmd[512];
    argc = 0;
    for(int i=0; i<20; i++) argv[i] = 0;
    while(1) {
        print("> ");
        read_input(cmd);
        parse_command(cmd);
        if (!cmd[0]) {
            print("\n");
            continue;
        }
        if (!strcmp(argv[0], "ver")) {
            print("Via64 Command Prompt Version 1.1\n");
        }
        if (!strcmp(argv[0], "help")) {
            print("ver, int, ; <filename> <arg>, echo <text>\n");
        }
        if (!strcmp(argv[0], "int")) {
            asm("int $0");
        }
        if (!strcmp(argv[0], ";")) {
            for(int i=0; i<64; i++) arg_backup[i] = 0;
            
            if (argc > 2) {
                for(int i=0; i<63 && argv[2][i]; i++) {
                    arg_backup[i] = argv[2][i];
                }
            }
            asm volatile (
                "int $0x80"
                : 
                : "a"(5),
                "b"(argv[1]),
                "d"(arg_backup)
                : "memory"
            );
        }
        if (!strcmp(argv[0], "echo")) {
            for(int i = 1; i < argc; i++) {
                print(argv[i]);
                print(" ");
            }
            print("\n");
        }
    }
}