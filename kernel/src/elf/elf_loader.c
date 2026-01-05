#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "elf.h"

#define ALIGN_UP(value, align) (((value) + (align) - 1) & ~((align) - 1))

uint64_t load_elf(void* file) {
    if (!verify_elf_64(file)) {
        write("ELF: Failed to load ELF file; VALIDATION FAILED\n", 2);
        return 0;
    }

    struct elf_header_64 *header = (struct elf_header_64 *)file;
    struct elf_program_header_64 *prog_headers = (struct elf_program_header_64 *)((uint8_t *)file + header->program_header_table);

    for (size_t i = 0; i < header->program_header_entries; i++) {
        struct elf_program_header_64 *ph = &prog_headers[i];

        if (ph->type != ELF_PROG_PT_LOAD_TYPE) continue;

        uint64_t start = ph->virt_addr;
        uint64_t end = start + ph->mem_size;
        uint64_t aligned_start = start & ~(PAGE_SIZE - 1);
        uint64_t aligned_end = ALIGN_UP(end, PAGE_SIZE);
        uint64_t pagesToMap = (aligned_end - aligned_start) / PAGE_SIZE;

        uint64_t pte_flags = PTE_PRESENT | PTE_USER | PTE_WRITABLE;
        if (ph->flags & ELF_PROG_WRITE) pte_flags |= PTE_WRITABLE;
        if (!(ph->flags & ELF_PROG_EXEC_FLAG)) pte_flags |= PTE_NX;

        uint64_t currentVirtAddr;
        if (ph->alignment > 1) {
            currentVirtAddr = ph->virt_addr & ~(ph->alignment - 1);
        } else {
            currentVirtAddr = ph->virt_addr;
        }

        for (uint64_t addr = aligned_start; addr < aligned_end; addr += PAGE_SIZE) {
            vmm_map(addr, (uint64_t)pmm_alloc(), pte_flags);
        }

        uint8_t* dest = (uint8_t*)ph->virt_addr;
        uint8_t* src  = (uint8_t*)file + ph->offset;
        memcpy(dest, src, ph->file_size);

        if (ph->mem_size > ph->file_size) {
            memset(dest + ph->file_size, 0, ph->mem_size - ph->file_size);
        }
    }

    return header->entry_point;
}
