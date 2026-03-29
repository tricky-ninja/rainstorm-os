OUTPUT_FORMAT(elf64-x86-64)
ENTRY(_start)

KERNEL_VMA = 0xFFFFFFFF80000000;

PHDRS
{
    limine_requests PT_LOAD FLAGS((1 << 2));
    text   PT_LOAD FLAGS((1 << 0) | (1 << 2));
    rodata PT_LOAD FLAGS((1 << 2));
    data   PT_LOAD FLAGS((1 << 1) | (1 << 2));
}

SECTIONS
{
    . = KERNEL_VMA;

    _kernel_start_virt = .;

    .limine_requests : {
        _LIMINE_BEGIN = .;
        KEEP(*(.limine_requests_start))
        KEEP(*(.limine_requests))
        KEEP(*(.limine_requests_end))
        _LIMINE_END = .;
    } :limine_requests

    . = ALIGN(CONSTANT(MAXPAGESIZE));

    .text : {
        _TEXT_BEGIN = .;
        *(.text*)
        _TEXT_END = .;
    } :text

    . = ALIGN(CONSTANT(MAXPAGESIZE));

    .rodata : {
        _RODATA_BEGIN = .;
        *(.rodata*)
        _shell_modules_start = .;
        KEEP(*(.shell_modules))
        _shell_modules_end = .;
        _RODATA_END = .;
    } :rodata

    . = ALIGN(CONSTANT(MAXPAGESIZE));

    .data : {
        _DATA_BEGIN = .;
        *(.data*)
    } :data

    .bss (NOLOAD) : {
        _bss_start = .;
        *(COMMON)
        *(.bss*)
        _bss_end = .;
        _DATA_END = .;
    } :data

    _kernel_end_virt = ALIGN(0x1000);
    KERNEL_SIZE      = _kernel_end_virt - KERNEL_VMA;
}