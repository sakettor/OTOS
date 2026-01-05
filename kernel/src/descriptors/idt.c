#include <stdint.h>
#include "../utils/utils.h"
#define BLACK 1
#define RED 2
#define GREEN 3
#define YELLOW 4
#define BLUE 5
#define WHITE 0

extern char user[20];

extern struct search {
    char* name;
    uint64_t size;
    char typeflag;
    char* data_address;
    char* header_address;
};

extern struct search tfs_search(char* filename);

/* idt here */

extern void *interruptTable[256];
extern void handle_keyboard();
void write(char* str, int col);
extern int read_stat;
extern struct flanterm_context *ft_ctx;
unsigned int lives = 30;
extern int rand_between(int min, int max);

char kb[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',   
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',   
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',   0,     
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',   0,   
    '*',    0,  ' ',    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    
    0,    0,    0,    0,    -1,     0,    0,    0,    0,    0,    0,    0,    0,    
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    
};

struct IDTEntry {
    uint16_t offset_low; 
    uint16_t selector;    
    uint8_t  ist;        
    uint8_t  type_attributes; 
    uint16_t offset_mid;  
    uint32_t offset_high;     
    uint32_t reserved;         
} __attribute__((packed));

struct IDTR {
    uint16_t limit;            
    uint64_t base;             
} __attribute__((packed));

struct IDTEntry idt[256];

struct interrupt_frame {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
    uint64_t intNo;
    uint64_t errorCode;
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
} __attribute__((packed));

struct saved_pos {
    uint64_t rip;
    uint64_t rsp;
};

volatile struct saved_pos int_saved_pos = {
    0,
    0
};

static char const* faultMessages[32] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Detected Overflow",
    "Out Of Bounds",
    "Invalid OpCode",
    "No CoProcessor",
    "Double Fault",
    "CoProcessor Segment Overrun",
    "Bad Tss",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",
    "CoProcessor Fault",
    "Alignment Check",
    "Machine Check",
    "Simd Floating Point Exception",
    "Virtualization Exception",
    "Control Protection Exception",
    "Reserved",
    "Hypervisor Injection Exception",
    "Vmm Communication Exception",
    "Security Exception",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
};

static char const* yourFaultMessages[] = {
    "..Why did you do that?",
    "So now we are sacrificing cpu cores for laughs huh??",
    "You need to go learn some etiquette!",
    "The kernel has been defeated.",
    "Aren't you the guy that bought Warner Brothers?",
    "Hey, dont be afraid, your system is not bricked.",
    "Who even wrote this stuff?",
    "You wanted a working system? The working system wants you.",
    "Hey stop eating my ram bitch!!",
    "What does this even mean?",
    "This system is so stable it overstabilized!",
    "No stack smashing for you.",
    "Wait i wanted the other cool error message!!",
    "Yeah no you messed up",
    "STOP GIVING ME UNREADABLE ERRORS!!!",
    "what",
    "My hard drive is a slave, literally.",
    "Noo, i forgot to remove the kernel killer app!",
    "I need to C A Therapist after this. (u see 'CAT' thats so funny lol uahah boom shakalaka)",
    "'Cool Exception Name'",
    "We dont allow nested errors. That is why you got the error!",
    "Mustard Six-Seven Exception",
    "Long live your CPU...",
    "Ah, no you cant access that.",
    "This shit is not even implemented yet!! How is it erroring??",
    "Ah, no you cant access that.",
    "Microsoft is loading their gun, probably",
    "Ah-hah! I found you, interrupt!",
    "'Wait nonono i have an unsaved document' <- you probably",
    "3 step guide to kill your operating system! Step 1, turn on the PC. Done!",
    "No CPU will live in my crashing kingdom!",
    "010111010001011010101",
    "No no no no... Its impossible, the OS is indestructible!",
    "what x2",
    "You made a little error, thats ok... Crash the kernel, burn the CPU, rewrite the bios!!!",
    "Coding in assembly is so satisf~",
    "HAHAHA LOL MOV RIP 67 HEEEHEE",
    "didnt you notice the little red light on your PC? that is called computer death",
    "oooh wait that means page fault... yeaaah thats bad",
    "Nine Floating Point Errors now available in Steam!",
    "buffer stack rip overstacking and underbuffering is so deadly",
    "wait i cant use printf() here?? wtf is this programming enviroment",
    "I AM MACHINE. I AM DELETE YOUR LAPTOP.",
    "dont worry triple faults wont kill your cpu",
    "i accidentaly used printf() didnt i",
    "what is this cpurot os omg",
    "When you try to launch 'kernel', your OS will not die! But it died because of this thingy down there! \\/",
    "your computer couldn't figure out if the following statement is false (Error code: 0x69FF57679)"
};

void fill_idt_struct(int index, void *handler) { // split the handler's address into 3 parts and fill in the IDTEntry struct
    uint64_t address = handler;
    uint16_t low = address & 0xFFFF;
    uint16_t mid = address >> 16 & 0xFFFF;
    uint32_t high = address >> 32;
    idt[index].ist = 0;
    idt[index].offset_high = high;
    idt[index].offset_low = low;
    idt[index].offset_mid = mid;
    idt[index].reserved = 0;
    idt[index].selector = 0x08; // our sigma gigachad gdt
    if (index == 0x80) {
        idt[index].type_attributes = 0xEE; 
    } else {
        idt[index].type_attributes = 0x8E;
    }
}

void io_wait(void) {
    outb(0x80, 0);
}

void remap_pic() {
    outb(0x20, 0x11);
    io_wait();
    outb(0xA0, 0x11);
    io_wait();
    outb(0x21, 0x20);
    io_wait();
    outb(0xA1, 0x28);
    io_wait();
    outb(0x21, 0x04);
    io_wait();
    outb(0xA1, 0x02);
    io_wait();
    outb(0x21, 0x01);
    io_wait();
    outb(0xA1, 0x01);
    io_wait();
    outb(0x21, 0xFF);
    outb(0xA1, 0xFF);
    io_wait();
}

void load_idt() {
    outb(0x21, 0xFC); 
    outb(0xA1, 0xFF);
    remap_pic();
    uint8_t mask = inb(0x21);
    outb(0x21, mask & ~(1 << 1)); // unmask irq1
    struct IDTR idtr = {
        sizeof(idt) - 1,
        (uint64_t)&idt
    };
    for (int i = 0; i < 256; i++) {
        fill_idt_struct(i, interruptTable[i]);
    }
    __asm__ volatile ("lidt %0" : : "m"(idtr));
    __asm__ volatile ("sti");
    outb(0x21, 0xFD); 
}

void write_hex(uint64_t val) {
    const char* hex = "0123456789ABCDEF";
    char buf[19];
    buf[0] = '0';
    buf[1] = 'x';
    for (int i = 15; i >= 0; i--) {
        buf[i + 2] = hex[val & 0xF];
        val >>= 4;
    }
    buf[18] = '\0';
    write(buf, RED);
}

void kernel_panic(struct interrupt_frame* frame, uint64_t cr2) {
    int msg_count = sizeof(yourFaultMessages) / sizeof(char*);
    write("\n~ ", RED);
    write(yourFaultMessages[rand_between(0, msg_count-1)], RED);
    write("\n---------------------------------------------------------------------\nYour system ran into an unrecoverable error.\nException: ", RED); 
    write(faultMessages[frame->intNo], RED); 
    write("\n\nRIP: ", RED); write_hex(frame->rip);
    write(" | ", 0);
    write("CS: ", RED); write_hex(frame->cs);
    write("\n", 0);
    write("RFLAGS: ", RED); write_hex(frame->rflags);
    write(" | ", 0);
    write("RSP: ", RED); write_hex(frame->rsp);
    write("\n", 0);
    write("SS: ", RED); write_hex(frame->ss);
    write(" | ", 0);
    write("CR2: ", RED); write_hex(cr2);
    write("\n", 0);
    write("RBX: ", RED); write_hex(frame->rbx);
    write(" | ", 0);
    write("RAX: ", RED); write_hex(frame->rax);
    write("\n", 0);
    write("Instruction at RIP: ", RED);
    for (int i = 0; i < 20; i++) {
        char byte_str[4];
        if (frame->rip < 0x1000 || frame->rip > 0xFFFFFFFFFFFFFFFF - 0x1000) {
            write(" (Address unreadable)", YELLOW);
            break;
        } else {
            uint8_t *rip_ptr = (uint8_t *)frame->rip;
            uint8_t byte = rip_ptr[i];
            const char hex_chars[] = "0123456789ABCDEF";
            byte_str[0] = hex_chars[(byte >> 4) & 0x0F];
            byte_str[1] = hex_chars[byte & 0x0F];
            byte_str[2] = ' ';
            byte_str[3] = '\0';
            write(byte_str, YELLOW);
        }
    }
    write("\nError code: ", RED); write_hex(frame->errorCode);
    hcf();
}

void exception(struct interrupt_frame* frame) {
    frame->cs = 0x08;
    frame->ss = 0x10;
    frame->rip = int_saved_pos.rip;
    frame->rsp = int_saved_pos.rsp;
    lives--;
    return;
}

void handleInterruptAsm(struct interrupt_frame* frame, uint64_t cr2) {
    if (frame->intNo < 32) {
        if (int_saved_pos.rip != 0 && lives != 0) {
            if (frame->cs == 27) {
                kernel_panic(frame, cr2);
                write("Oopsie!\nSegmentation fault.\nFaulty instruction - ", YELLOW);
                uint8_t *rip_ptr = (uint8_t *)frame->rip;
                for (int i = 0; i < 10; i++) {
                    char byte_str[4];
                    uint8_t byte = rip_ptr[i];
                    const char hex_chars[] = "0123456789ABCDEF";
                    byte_str[0] = hex_chars[(byte >> 4) & 0x0F];
                    byte_str[1] = hex_chars[byte & 0x0F];
                    byte_str[2] = ' ';
                    byte_str[3] = '\0';
                    write(byte_str, YELLOW);
                }
                write("\n", 0);
                int result = tfs_launch("cmd.bin");
    
                if (result != 0) {
                    exception(frame);
                }
                return;
            }
            exception(frame);
            return;
        }
        kernel_panic(frame, cr2);
    } else {
        if (frame->intNo == 0x80) {
            switch (frame->rax) {
                case 1:
                    if (frame->rbx < 0x1000) {
                        write("kernel: tried to print from invalid pointer\n", RED);
                    } else {
                        write((char*)frame->rbx, 0);
                    }
                    break;
                case 2:
                    char* command_buf = (char*)frame->rbx;
                    int cursor_pos = 0;
                    while (1) {
                        unsigned char status = inb(0x64);

                        if (status & 0x01) {
                            unsigned char scan_code = inb(0x60);

                            if (scan_code & 0x80) {
                                continue; 
                            }
                            
                            if (scan_code < 60) {
                                char ascii = kb[scan_code];
                                
                                if (ascii != 0) {
                                    if (ascii == '\b') {
                                        if (cursor_pos > 0) {
                                            write("\b \b", WHITE);
                                            cursor_pos--;
                                            command_buf[cursor_pos] = 0;
                                        }
                                    } else
                                    if (ascii == '\n') {
                                        command_buf[cursor_pos] = '\0';
                                        write("\n", 0);
                                        __asm__ volatile (
                                            ""
                                            : 
                                            : "a"(2), "b"(command_buf)
                                            : "memory"
                                        );
                                        break;
                                    } else {
                                        flanterm_write(ft_ctx, &ascii, 1);
                                        command_buf[cursor_pos++] = ascii;
                                    }
                                }
                            }
                        }
                    }
                    break;
                case 3: // save
                    if (frame->cs == 0x08) {
                        int_saved_pos.rip = frame->rip;
                        int_saved_pos.rsp = frame->rsp;
                        lives = 30;
                    }
                    break;
                case 4: // eggsit
                    char code[512];
                    int_to_str(frame->rbx, code);
                    write("Program exited with code ", 0);
                    if (frame->rbx == 0) {
                        write(code, GREEN);
                    } else {
                        write(code, RED);
                    }
                    write("\n", 0);
                    tfs_launch("cmd");
                    break;
                case 5: // launch
                    int result = tfs_launch((char*)frame->rbx, (char*)frame->rdx);
                    if (result == 0) {
                        write("File not found.\n", RED);
                    }
                    break;
                case 6: // read
                    int fd        = (int)frame->rbx;
                    char* user_buf = (char*)frame->rdx;
                    int count     = (int)frame->rcx;
                    if ((uint64_t)user_buf >= 0xFFFF800000000000) {
                        frame->rax = 0;
                        break;
                    }
                    frame->rax = tfs_read(fd, user_buf, count);
                    break;
                case 7: // dir
                    tfs_dir();
                    break;
                case 8: // allocation
                    frame->rax = kmalloc(frame->rbx);
                    break;
                case 9: // file open
                    frame->rax = tfs_open((char*)frame->rbx);
                    break;
                case 10: // file close
                    frame->rax = tfs_close(frame->rbx);
                    break;
                case 11: // seek
                    frame->rax = tfs_seek(frame->rbx, frame->rdx);
                    break;
                case 12: // tcb set
                    frame->rax = 0;
                    uint64_t fs_base = frame->rbx;
                    write("Setting TCB base to: ", 0);
                    write_hex(fs_base);
                    if (fs_base == 0) {
                        break;
                    }

                    uint32_t low = (uint32_t)fs_base;
                    uint32_t high = (uint32_t)(fs_base >> 32);
                    __asm__ volatile ("wrmsr" : : "a"(low), "d"(high), "c"(0xC0000100) : "memory");
                    
                    frame->rax = 0;
                    write("\n", 0);
                    break;
                case 13: // syscall 1 but better :D | rbx - fd, rdx - buffer, rcx - size
                    if (frame->rbx == 1 || frame->rbx == 2) {
                        flanterm_write(ft_ctx, (char *)frame->rdx, frame->rcx);
                        frame->rax = frame->rcx; 
                    } else {
                        frame->rax = 0;
                    }
                    break;
                case 14: // user operations
                    if (frame->rbx == 1) { // get username
                        frame->rax = (char*)user;
                    } else if (frame->rbx == 2) { // change username
                        memcpy(user, frame->rdx, 19);
                        frame->rax = (char*)user;
                    }
                    break;
            }
        }
    }
}
