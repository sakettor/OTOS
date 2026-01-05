CFLAGS="-m64 -c -nostdlib -ffreestanding -fno-stack-protector -mno-red-zone -fno-plt -mcmodel=small"
LD_FLAGS="-m elf_x86_64 -static -T user.ld"
rm rootfs.tar
cd programs
as --64 ./hello.S -o hello.o
ld -m elf_x86_64 hello.o -o hello
cp hello ../tar-files
x86_64-elf-gcc $CFLAGS lib.c -o lib.o
x86_64-elf-gcc $CFLAGS cmd.c -o cmd.o
x86_64-elf-gcc $CFLAGS check.c -o check.o
x86_64-elf-gcc $CFLAGS hexdump.c -o hexdump.o
x86_64-elf-gcc $CFLAGS type.c -o type.o
x86_64-elf-gcc $CFLAGS otfetch.c -o otfetch.o
x86_64-elf-gcc $CFLAGS dir.c -o dir.o
nasm -f elf64 entry.S -o entry.o
x86_64-elf-ld $LD_FLAGS entry.o cmd.o lib.o -o cmd
x86_64-elf-ld $LD_FLAGS entry.o hexdump.o lib.o -o hexdump
x86_64-elf-ld $LD_FLAGS entry.o check.o lib.o -o check
x86_64-elf-ld $LD_FLAGS entry.o type.o lib.o -o type
x86_64-elf-ld $LD_FLAGS entry.o otfetch.o lib.o -o otfetch
x86_64-elf-ld $LD_FLAGS entry.o dir.o lib.o -o dir
cp cmd ../tar-files
cp check ../tar-files
cp hexdump ../tar-files
cp type ../tar-files
cp otfetch ../tar-files
cp dir ../tar-files
rm cmd
rm hexdump
rm type
rm otfetch
rm dir
rm check
rm hello
rm *.o
tar --format=ustar -cvf ../rootfs.tar -C ../tar-files/ .