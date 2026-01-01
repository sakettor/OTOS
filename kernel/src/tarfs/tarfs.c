#include <stdint.h>

extern uint64_t tar_address;
extern char user_stack[4096];
extern void usermode_entry(uint64_t rip, uint64_t rsp, uint64_t arg_ptr);

int oct2int(const char *str, int size) {
    int val = 0;
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

int tfs_launch(char* filename, char* arg) {
    struct search app = tfs_search(filename);
    if (app.data_address != 0) {
        char* arg_location = (char*)user_stack;
        for(int i=0; i<64; i++) arg_location[i] = 0;
        for (uintptr_t p = (uintptr_t)app.data_address; p < (uintptr_t)app.data_address + app.size + 8192; p += 4096) {
            make_user_accessible(p);
        }
        
        for (uintptr_t p = (uintptr_t)user_stack; p < (uintptr_t)user_stack + sizeof(user_stack); p += 4096) {
            make_user_accessible(p);
        }
        for(int i = 0; i < 63 && arg[i]; i++) {
            arg_location[i] = arg[i];
            arg_location[i+1] = '\0';
        }

        usermode_entry((uint64_t)app.data_address, 
                       (uint64_t)user_stack + sizeof(user_stack) - 128,
                       (uint64_t)arg_location);
        return 0;
    }
    return 1;
}