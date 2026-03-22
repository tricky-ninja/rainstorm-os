OUTPUT_FORMAT(elf32-i386)
ENTRY(_start)

KERNEL_VMA = 0xC0000000;

SECTIONS
{
    . = KERNEL_VMA + 1M;
    _kernel_start_virt = .;
    _kernel_start_phys = . - KERNEL_VMA;

    .multiboot ALIGN(0x1000) : AT(ADDR(.multiboot) - KERNEL_VMA)  {
        *(.multiboot)
    }
    
    .text ALIGN(0x1000) : AT(ADDR(.text) - KERNEL_VMA) {
        *(.text)
    }

    .rodata ALIGN(0x1000) : AT(ADDR(.rodata) - KERNEL_VMA) {
        *(.rodata)
    }

    .shell_modules ALIGN(0x1000) : AT(ADDR(.shell_modules) - KERNEL_VMA) {
        _shell_modules_start = .;
        KEEP(*(.shell_modules))
        _shell_modules_end = .;
    }

    .data ALIGN(0x1000) : AT(ADDR(.data) - KERNEL_VMA) {
        *(.data)
    }

    .bss ALIGN(0x1000)  (NOLOAD) : AT(ADDR(.bss) - KERNEL_VMA) {
        _bss_start = .;
        *(COMMON)
        *(.bss)
        _bss_end = .;
    }

    _kernel_end_virt = ALIGN(0x1000);
    _kernel_end_phys = _kernel_end_virt - KERNEL_VMA;
}