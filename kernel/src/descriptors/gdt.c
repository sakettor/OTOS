#include <stdint.h>
#include "../utils/utils.h"

char kernel_stack[16384];
char *user_stack;

extern void gdt_reload();
extern void usermode_entry(uint64_t aeeeh, uint64_t idkkk);
extern void hello_sim();
extern uint64_t HHDM_OFFSET;
extern void write(char* str, int col);
extern uint64_t fb_size;
extern uint64_t fb_addr;
extern uint64_t kmalloc(uint64_t incr);
uint64_t user_stack_top = 0;

uint64_t rsp0_top = (uint64_t)kernel_stack + sizeof(kernel_stack);

struct gdt_ent {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
} __attribute__((packed));

struct gdt_tss_ent {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
    uint32_t base_upper;
    uint32_t reserved;
} __attribute__((packed));

struct gdtr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

struct tss {
    uint32_t reserved0;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t reserved1;
    uint64_t ist1;
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t iopb_offset;
} __attribute__((packed));

struct GDTR {
    uint16_t limit;            
    uint64_t base;             
} __attribute__((packed));

union gdt_entry_union {
    struct gdt_ent fields;
    uint64_t raw;
};

union gdt_entry_union gdt[7];

struct tss tsSs = {0};

void shell() {
    char input[500];
    while(1) {
        __asm__ volatile ("int $0x80" : : "a"(2), "b"(input) : "rcx", "r11", "memory");

        __asm__ volatile ("int $0x80" : : "a"(1), "b"(input) : "rcx", "r11", "memory");
        
        __asm__ volatile ("int $0x80" : : "a"(1), "b"("\n") : "rcx", "r11", "memory");
    }
}

void make_user_accessible(uintptr_t virt_addr) {
    uintptr_t cr3_phys;
    __asm__ volatile("mov %%cr3, %0" : "=r"(cr3_phys));

    uint64_t* current_table = (uint64_t*)(cr3_phys + HHDM_OFFSET);

    for (int level = 39; level >= 12; level -= 9) {
        int index = (virt_addr >> level) & 0x1FF;
        uint64_t entry = current_table[index];

        current_table[index] |= (1 << 2);

        if (level == 12 || (entry & (1 << 7))) {
            break; 
        }

        uintptr_t next_table_phys = current_table[index] & 0x000FFFFFFFFFF000;
        current_table[index] |= (1 << 0); // present
        current_table[index] |= (1 << 1); // writable
        current_table[index] |= (1 << 2);
        
        current_table = (uint64_t*)(next_table_phys + HHDM_OFFSET);
    }

    __asm__ volatile("mov %%cr3, %%rax\nmov %%rax, %%cr3" ::: "rax");
}

void gdt_init() {
    user_stack = kmalloc(16384);
    user_stack_top = (uint64_t)user_stack + 16384;
    gdt[0].raw = 0x0000000000000000; // null
    gdt[1].raw = 0x00af9b000000ffff; // kernel code
    gdt[2].raw = 0x00af93000000ffff; // kernel data
    gdt[3].raw = 0x00affb000000ffff; // user code
    gdt[4].raw = 0x00aff3000000ffff; // user data

    tsSs.rsp0 = rsp0_top;
    tsSs.iopb_offset = sizeof(struct tss);

    uint64_t addr = (uint64_t)&tsSs;

    struct gdt_tss_ent tss_e = {
        sizeof(struct tss) - 1,
        addr & 0xFFFF,
        (addr >> 16) & 0xFF,
        0x89,
        0x00,
        (addr >> 24) & 0xFF,
        (addr >> 32) & 0xFFFFFFFF,
        0
    };

    struct GDTR gdtr = {
        sizeof(gdt) - 1,
        (uint64_t)&gdt
    };
    *(struct gdt_tss_ent*)&gdt[5] = tss_e;
    __asm__ volatile ("lgdt %0" : : "m"(gdtr));
    gdt_reload();
}