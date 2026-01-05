#include <stdint.h>
#include "../utils/utils.h"
#define MAX_FILES 128

extern uint64_t tar_address;
extern uint64_t user_stack_top;
extern char *user_stack;
extern void usermode_entry(uint64_t rip, uint64_t rsp, uint64_t arg_ptrm);
extern uint64_t load_elf(void* file);

typedef struct file {
    char name[100];
    uint64_t data_address;
    int size;
    int seek;
} file;

file *file_descriptors;

int oct2int(const char *str, int size) {
    int val = 0;
    while (size > 0 && (*str == ' ' || *str == '\0')) {
        str++;
        size--;
    }
    while (size-- > 0 && *str >= '0' && *str <= '7') {
        val = (val << 3) | (*str++ - '0');
    }
    return val;
}

struct search {
    char* name;
    uint64_t size;
    char typeflag;
    char* data_address;
    char* header_address;
};

struct tar_header {
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char chksum[8];
    char typeflag;
    char linkname[100];
    char magic[6];
    char version[2];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
    char prefix[155];
};

struct search tfs_search(char* filename) {
    int cursor = 0;
    int len = 0;
    char fname_buf[103] = "./";
    while(filename[len] != 0 && len < 200) len++; 

    if (len > 100) {
        return (struct search){0};
    }

    strcat(fname_buf, filename);
    struct tar_header *ptr;
    char* header;
    char* data_address;
    uint64_t data_len = 0;

    for (;;) {
        header = (struct tar_header *)(tar_address + cursor);
        cursor += 512;
        ptr = (struct tar_header *)header;
        if (ptr->name[0] == 0) {
            break;
        }
        int filesize = oct2int(ptr->size, 12);
        int datasize = (filesize + 511) / 512 * 512;
        if (!strcmp(fname_buf, ptr->name)) {
            if (ptr->typeflag == '5') {
                break;
            }
            data_address = tar_address + cursor;
            data_len = datasize;
            struct search result = {
                filename,
                filesize,
                0,
                data_address,
                header
            };
            return result;
            break;
        }
        cursor += datasize;
    }
    struct search result = {
        0,
        0,
        0,
        0,
        0
    };
    return result;
}

void tfs_fd_init() {
    uint64_t size = sizeof(file) * MAX_FILES;
    file_descriptors = (file*)bmp_alloc(size);
    memset(file_descriptors, 0, size);
}

int tfs_open(char *filename) {
    struct search results = tfs_search(filename);
    if (results.data_address == 0) {
        return -1;
    }
    for (int i = 0; i < MAX_FILES; i++) {
        if (file_descriptors[i].data_address == 0) {
            strcpy(file_descriptors[i].name, filename);
            file_descriptors[i].data_address = results.data_address;
            file_descriptors[i].size = results.size;
            file_descriptors[i].seek = 0;
            return i;
        }
    }
}

int tfs_close(int fd) {
    if (file_descriptors[fd].data_address == 0) {
        return -1;
    }
    memset(file_descriptors[fd].name, 0, 100);
    file_descriptors[fd].data_address = 0;
    file_descriptors[fd].size = 0;
    file_descriptors[fd].seek = 0;
    return 0;
}

int tfs_read(int fd, char *buffer, int n) {
    if (fd >= 128 || fd < 0) {
        return -1;
    }
    if (file_descriptors[fd].data_address == 0) {
        return -1;
    }
    if (file_descriptors[fd].seek + n > file_descriptors[fd].size) {
        n = file_descriptors[fd].size - file_descriptors[fd].seek;
    }
    if (n < 0) return 0;
    int old_seek = file_descriptors[fd].seek;
    file_descriptors[fd].seek = file_descriptors[fd].seek + n;
    memcpy(buffer, file_descriptors[fd].data_address + old_seek, n);
    return n;
}

void tfs_seek(int fd, int offset) {
    if (fd >= 128 || fd < 0) {
        return;
    }
    if (file_descriptors[fd].data_address == 0) {
        return;
    }
    if (offset < 0 || offset > file_descriptors[fd].size) {
        return;
    }
    file_descriptors[fd].seek += offset;
}

struct search tfs_dir() {
    int cursor = 0;
    struct tar_header *ptr;
    char* header;
    char* data_address;
    uint64_t data_len = 0;

    for (;;) {
        header = (struct tar_header *)(tar_address + cursor);
        cursor += 512;
        ptr = (struct tar_header *)header;
        if (ptr->name[0] == 0) {
            break;
        }
        write(ptr->name, 0);
        write("\n");
        int filesize = oct2int(ptr->size, 12);
        int datasize = (filesize + 511) / 512 * 512;
        cursor += datasize;
    }
}

int tfs_launch(char* filename, char* arg) {
    struct search app = tfs_search(filename);
    if (app.data_address == 0) return 1;

    uint64_t entry = load_elf(app.data_address);
    if (entry == 0 || entry > 0xFFFFFFFF80000000) return 2;

    char* u_arg_str = kmalloc(64);
    memset(u_arg_str, 0, 64);
    memcpy(u_arg_str, arg, 63);
    
    usermode_entry(entry, (uint64_t)user_stack_top, (uint64_t)u_arg_str);
    
    return 0;
}