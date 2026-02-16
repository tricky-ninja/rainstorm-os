OUTPUT_FORMAT(elf32-i386)
ENTRY(_start)

SECTIONS
{
    . = 1M;
    _kernel_start = .;

    .multiboot ALIGN(0x1000) :  {
        *(.multiboot)
    }
    
    .text ALIGN(0x1000) : {
        *(.text)
    }

    .rodata ALIGN(0x1000) : {
        *(.rodata)
    }

    .data ALIGN(0x1000) : {
        *(.data)
    }

    .bss ALIGN(0x1000)  (NOLOAD) : {
        _bss_start = .;
        *(.bss)
        _bss_end = .;
    }

    _kernel_end = ALIGN(0x1000);
}