#include <stdint.h>
#include <stdbool.h>

extern uint64_t HHDM_OFFSET;
#define PTE_PRESENT  (1ULL << 0)
uint64_t heap_start = 0x10000000;
uint64_t heap = 0;
uint64_t limit = 0;

uint64_t* get_pte(uintptr_t virt_addr, bool create) {
    uintptr_t cr3_phys;
    __asm__ volatile("mov %%cr3, %0" : "=r"(cr3_phys));

    uint64_t* current_table = (uint64_t*)(cr3_phys + HHDM_OFFSET);

    for (int level = 39; level > 12; level -= 9) {
        int index = (virt_addr >> level) & 0x1FF;
        
        if (!(current_table[index] & 1)) {
            if (!create) return 0;

            uint64_t new_table_phys = pmm_alloc();
            memset((void*)(new_table_phys + HHDM_OFFSET), 0, 4096);

            current_table[index] = new_table_phys | 0b111; 
        }

        uintptr_t next_table_phys = current_table[index] & 0x000FFFFFFFFFF000;
        current_table = (uint64_t*)(next_table_phys + HHDM_OFFSET);
    }

    int final_index = (virt_addr >> 12) & 0x1FF;
    return &current_table[final_index];
}

void vmm_map(uintptr_t virt, uintptr_t phys, uint64_t flags) {
    uint64_t* pte = get_pte(virt, true);
    if (*pte & PTE_PRESENT) {
        return; 
    }
    *pte = phys | flags;
    __asm__ volatile("invlpg (%0)" : : "r"(virt) : "memory");
}

uint64_t bmp_alloc(uint64_t incr) {
    uint64_t old_heap = heap;
    heap = heap + incr;
    if (heap < limit) {
        return old_heap;
    }
    uint64_t new_limit = (heap + 4095) / 4096 * 4096;
    for (uint64_t i = limit; i < new_limit; i+=4096) {
        uint64_t addr = pmm_alloc();
        vmm_map(i, addr, 0x3);
    }
    limit = new_limit;
    return old_heap;
}