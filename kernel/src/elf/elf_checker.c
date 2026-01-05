#include "elf.h"
#include <stdbool.h>

bool verify_elf_magic(struct elf_header_64 *header) {
    return header->magic[0] == 0x7F &&
           header->magic[1] == 'E' &&
           header->magic[2] == 'L' &&
           header->magic[3] == 'F';
}

bool verify_elf_64(void* file) {
    struct elf_header_64 *header = (struct elf_header_64 *)file;
    if (verify_elf_magic(header)) {
        // write("ELF: Magic Value Good\n");
    } else {
        write("ELF: BAD ELF FILE\n", 2);
        return false;
    }

    if (header->header_version != ELF_HEADER_VER) {
        write("ELF: BAD ELF FILE\n", 2);
        return false;
    }

    if (header->elf_version != ELF_VER) {
        write("ELF: BAD ELF FILE\n", 2);
        return false;
    }

    if (header->bits == ELF_64_BIT) {
        // write("ELF: 64 Bit Elf File\n");
    } else if (header->bits == ELF_32_BIT) {
        write("ELF: 32 Bit Elf File; Not Supported\n", 2);
    } else {
        write("ELF: BAD ELF FILE\n", 2);
        return false;
    }

    if (header->endianness == ELF_LITTLE_ENDIAN) {
        // write("ELF: Little Endian Elf File\n");
    } else if (header->endianness == ELF_BIG_ENDIAN) {
        write("ELF: Big Endian Elf File; Not Supported\n", 2);
        return false;
    } else {
        write("ELF: BAD ELF FILE\n", 2);
        return false;
    }

    if (header->arch == ELF_X86_64_ARCH) {
        // write("ELF: x86-64 Detected\n");
    } else {
        write("ELF: Unkown Arch Detected; Not Supported\n", 2);
        return false;
    }

    if (header->abi == ELF_SYSV_ABI) {
        // write("ELF: SYSV Detected\n");
    } else {
        write("ELF: Unkown Arch Detected; Not Supported\n", 2);
        return false;
    }

    if (header->type == ELF_EXECUTABLE_TYPE) {
        // write("ELF: Executable Detected\n");
    } else {
        write("ELF: Unkown Type Detected; Not Supported\n", 2
        );
        return false;
    }

    return true;
}