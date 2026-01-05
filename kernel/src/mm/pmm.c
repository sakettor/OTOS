#include <stdint.h>
#include <limine.h>

#define BLACK 1
#define RED 2
#define GREEN 3
#define YELLOW 4
#define BLUE 5
#define WHITE 0

uint8_t* bmap_virt_addr = 0;
uint64_t bmap_in_bytes = 0;

extern uint64_t HHDM_OFFSET;
extern volatile struct limine_memmap_request memmap;

uint64_t pmm_alloc() {
    for (int i = 0; i < bmap_in_bytes; i++) {
        if (bmap_virt_addr[i] == 0xFF) {
            continue;
        }
        for (int o = 0; o < 8; o++) {
            int bit = (bmap_virt_addr[i] >> o) & 1;
            if (bit == 0) {
                int page = (i * 8) + o;
                bitmap_set(page);
                uint64_t phys_addr = page * 4096;
                return phys_addr;
            }
        }
    }
}

void pmm_free(void* addr) {
    uint64_t phys_addr = (uint64_t)addr - HHDM_OFFSET;
    int page_index = phys_addr / 4096;
    bitmap_clear(page_index);
}

void bitmap_set(int page) {
    int byte = page / 8;
    int bit = page % 8;
    bmap_virt_addr[byte] |= (1 << bit);
}

void bitmap_clear(int page) {
    int byte = page / 8;
    int bit = page % 8;
    bmap_virt_addr[byte] &= ~(1 << bit);
}

void bitmap_init() {
    uint64_t bmap_phys_addr = 0;
    uint64_t count = memmap.response->entry_count;
    uint64_t highest_address = 0;
    char str[50];
    for (uint64_t i = 0; i < count; i++) {
    
        struct limine_memmap_entry *entry = memmap.response->entries[i];

        if (entry->type == LIMINE_MEMMAP_USABLE || entry->type == LIMINE_MEMMAP_EXECUTABLE_AND_MODULES || entry->type == LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE) {
            uint64_t end_of_chunk = entry->base + entry->length;

            if (end_of_chunk > highest_address) {
                highest_address = end_of_chunk;
            }
        } 
    }
    int_to_str(highest_address, str);
    write("Found RAM: ", GREEN);
    write(str, GREEN);
    write(" bytes\n", GREEN);
    bmap_in_bytes = highest_address / 4096;
    bmap_in_bytes = (bmap_in_bytes + 7) / 8;
    for (uint64_t i = 0; i < count; i++) {
    
        struct limine_memmap_entry *entry = memmap.response->entries[i];

        if (entry->type == LIMINE_MEMMAP_USABLE && entry->length > bmap_in_bytes) {
            write("Found usable RAM for bitmap allocator!\n", GREEN);
            bmap_phys_addr = entry->base;
            break;
        } 
    }
    bmap_virt_addr = bmap_phys_addr + HHDM_OFFSET;
    memset(bmap_virt_addr, 0xFF, bmap_in_bytes);
    write("Initialized bitmap.\n", GREEN);
    for (uint64_t i = 0; i < count; i++) {
    
        struct limine_memmap_entry *entry = memmap.response->entries[i];

        if (entry->type == LIMINE_MEMMAP_USABLE) {
            int start_page = entry->base / 4096;
            int times = entry->length / 4096;
            for (int o = 0; o < times; o++) {
                bitmap_clear(start_page + o);
            }
        } 
    }
    int bitmap_page = bmap_phys_addr / 4096;
    int bitmap_in_pages = (bmap_in_bytes + 4095) / 4096;
    for (int o = 0; o < bitmap_in_pages; o++) {
        bitmap_set(bitmap_page + o);
    }
    bitmap_set(0);
}