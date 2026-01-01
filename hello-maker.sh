rm rootfs.tar
cd programs
as --64 ./hello.S -o hello.o
ld -m elf_x86_64 -Ttext 0x0 --oformat binary hello.o -o hello
cp hello ../tar-files
../../opt/cross/bin/x86_64-elf-gcc -m64 -c lib.c -o lib.o -nostdlib -ffreestanding -fno-stack-protector -mno-red-zone -fPIC -fpie
../../opt/cross/bin/x86_64-elf-gcc -m64 -c cmd.c -o cmd.o -nostdlib -ffreestanding -fno-stack-protector -mno-red-zone -fPIC -fpie
../../opt/cross/bin/x86_64-elf-gcc -m64 -c hexdump.c -o hexdump.o -nostdlib -ffreestanding -fno-stack-protector -mno-red-zone -fPIC -fpie
../../opt/cross/bin/x86_64-elf-gcc -m64 -c type.c -o type.o -nostdlib -ffreestanding -fno-stack-protector -mno-red-zone -fPIC -fpie
../../opt/cross/bin/x86_64-elf-gcc -m64 -c otfetch.c -o otfetch.o -nostdlib -ffreestanding -fno-stack-protector -mno-red-zone -fPIC -fpie
../../opt/cross/bin/x86_64-elf-gcc -m64 -c dir.c -o dir.o -nostdlib -ffreestanding -fno-stack-protector -mno-red-zone -fPIC -fpie
nasm -f elf64 entry.S -o entry.o
ld -m elf_x86_64 -Ttext 0x0 --oformat binary entry.o cmd.o lib.o -o cmd
ld -m elf_x86_64 -Ttext 0x0 --oformat binary entry.o hexdump.o lib.o -o hexdump
ld -m elf_x86_64 -Ttext 0x0 --oformat binary entry.o type.o lib.o -o type
ld -m elf_x86_64 -Ttext 0x0 --oformat binary entry.o otfetch.o lib.o -o otfetch
ld -m elf_x86_64 -Ttext 0x0 --oformat binary entry.o dir.o lib.o -o dir
cp cmd ../tar-files
cp hexdump ../tar-files
cp type ../tar-files
cp otfetch ../tar-files
cp dir ../tar-files
tar --format=ustar -cvf ../rootfs.tar -C ../tar-files/ .