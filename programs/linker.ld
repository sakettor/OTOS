OUTPUT_FORMAT(binary)
ENTRY(_start)
SECTIONS
{
    . = 0x0; /* Твои бинарники грузятся по адресу из TAR, поэтому смещение 0 */
    .text : {
        *(.text.main) /* Сначала точка входа */
        *(.text)
    }
    .data : { *(.data) }
    .bss  : { *(.bss)  }
}