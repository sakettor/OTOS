#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>
#include "ft/flanterm.h"
#include "ft/flanterm_backends/fb.h"
#include "font/font.h"
#include "descriptors/idt.h"
struct flanterm_context *ft_ctx = NULL;
#define BLACK 1
#define RED 2
#define GREEN 3
#define YELLOW 4
#define BLUE 5
#define WHITE 0
extern struct search {
    char* name;
    uint64_t size;
    char typeflag;
    char* data_address;
    char* header_address;
};
extern struct search tfs_search();

uint64_t tar_address = 0;

// Set the base revision to 4, this is recommended as this is the latest
// base revision described by the Limine boot protocol specification.
// See specification for further info.

__attribute__((used, section(".limine_requests")))
static volatile uint64_t limine_base_revision[] = LIMINE_BASE_REVISION(4);

__attribute__((used, section(".limine_requests")))
static volatile struct limine_module_request module_request = {
    .id = LIMINE_MODULE_REQUEST_ID,
    .revision = 0
};

// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimise them away, so, usually, they should
// be made volatile or equivalent, _and_ they should be accessed at least
// once or marked as used with the "used" attribute as done here.

__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_hhdm_request hhdm = {
    .id = LIMINE_HHDM_REQUEST_ID,
    .revision = 0
};

// Finally, define the start and end markers for the Limine requests.
// These can also be moved anywhere, to any .c file, as seen fit.

__attribute__((used, section(".limine_requests_start")))
static volatile uint64_t limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end")))
static volatile uint64_t limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;

// GCC and Clang reserve the right to generate calls to the following
// 4 functions even if they are not directly called.
// Implement them as the C specification mandates.
// DO NOT remove or rename these functions, or stuff will eventually break!
// They CAN be moved to a different .c file.

void *memcpy(void *restrict dest, const void *restrict src, size_t n) {
    uint8_t *restrict pdest = (uint8_t *restrict)dest;
    const uint8_t *restrict psrc = (const uint8_t *restrict)src;

    for (size_t i = 0; i < n; i++) {
        pdest[i] = psrc[i];
    }

    return dest;
}

void *memset(void *s, int c, size_t n) {
    uint8_t *p = (uint8_t *)s;

    for (size_t i = 0; i < n; i++) {
        p[i] = (uint8_t)c;
    }

    return s;
}

void *memmove(void *dest, const void *src, size_t n) {
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;

    if (src > dest) {
        for (size_t i = 0; i < n; i++) {
            pdest[i] = psrc[i];
        }
    } else if (src < dest) {
        for (size_t i = n; i > 0; i--) {
            pdest[i-1] = psrc[i-1];
        }
    }

    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    const uint8_t *p1 = (const uint8_t *)s1;
    const uint8_t *p2 = (const uint8_t *)s2;

    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] < p2[i] ? -1 : 1;
        }
    }

    return 0;
}

// Halt and catch fire function.
static void hcf(void) {
    for (;;) {
#if defined (__x86_64__)
        asm ("hlt");
#elif defined (__aarch64__) || defined (__riscv)
        asm ("wfi");
#elif defined (__loongarch64)
        asm ("idle 0");
#endif
    }
}

void write_n(char* str, int n) {
    flanterm_write(ft_ctx, str, n);
}

void write(char* str, int col) {
    char* color_table[] = {
        "\033[37m", "\033[30m", "\033[31m", "\033[32m", 
        "\033[33m", "\033[34m"
    };

    if (col >= 0 && col <= 6) {
        flanterm_write(ft_ctx, color_table[col], 5);
    }
    
    for (int i = 0; str[i] != '\0' && i < 500; i++) {
        flanterm_write(ft_ctx, &str[i], 1);
    }
}

uint64_t HHDM_OFFSET;
uint64_t fb_size = 0;
uint64_t fb_addr = 0;

static inline uint64_t read_tsc() {
    uint32_t hi, lo;
    __asm__ volatile ("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
}

uint64_t xorshift64star() {
    static uint64_t x = 0;
    if (x == 0) {
        x = read_tsc();
    }

    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    return x * 0x2545F4914F6CDD1DULL;
}

int rand_between(int min, int max) {
    if (min > max) {
        int tmp = min;
        min = max;
        max = tmp;
    }

    uint64_t range = (uint64_t)(max - min + 1);
    uint64_t x, limit;

    limit = UINT64_MAX - (UINT64_MAX % range);

    do {
        x = xorshift64star();
    } while (x >= limit);

    return min + (x % range);
}

// The following will be our kernel's entry point.
// If renaming kmain() to something else, make sure to change the
// linker script accordingly.
void kmain(void) {
    // Ensure the bootloader actually understands our base revision (see spec).
    if (LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision) == false) {
        hcf();
    }

    // Ensure we got a framebuffer.
    if (framebuffer_request.response == NULL
     || framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];
    uint32_t *framebuffer_ptr = (uint32_t *)framebuffer->address;
    size_t width  = (size_t)framebuffer->width;
    size_t height = (size_t)framebuffer->height;
    size_t pitch  = (size_t)framebuffer->pitch;
    uint8_t red_mask_size   = framebuffer->red_mask_size;
    uint8_t red_mask_shift  = framebuffer->red_mask_shift;
    uint8_t green_mask_size = framebuffer->green_mask_size;
    uint8_t green_mask_shift= framebuffer->green_mask_shift;
    uint8_t blue_mask_size  = framebuffer->blue_mask_size;
    uint8_t blue_mask_shift = framebuffer->blue_mask_shift;
    fb_size = framebuffer->height * framebuffer->pitch;
    uint32_t festive_bg = 0x000044;
    uint32_t festive_fg = 0xFFFF00;

    ft_ctx = flanterm_fb_init(
        NULL,
        NULL,
        framebuffer_ptr, width, height, pitch,
        red_mask_size, red_mask_shift,
        green_mask_size, green_mask_shift,
        blue_mask_size, blue_mask_shift,
        NULL,
        NULL, NULL,
        &festive_bg,
        NULL,
        NULL, NULL,
        terminal_font, 8, 14, 1,
        1, 1,
        0, FLANTERM_FB_ROTATE_0
    );
    HHDM_OFFSET = hhdm.response->offset;
    fb_addr = framebuffer->address;
    load_idt();
    gdt_init();
    tar_address = module_request.response->modules[0]->address;
    for (int i = 0; i < 3000; i++) {
        int star_x = rand_between(0, 585456) % width;
        int star_y = rand_between(0, 4858934) % height;
        // Draw a 1x1 white pixel (Star)
        framebuffer_ptr[star_y * (pitch / 4) + star_x] = 0xFFFFFF;
    }
    write("OTos booted. Launching command prompt...\n", WHITE);
    __asm__ volatile ("int $0x80" : : "a"(3) : "rcx", "r11", "memory");
    tfs_launch("cmd", "");
    // We're done, just hang...
    hcf();
}
