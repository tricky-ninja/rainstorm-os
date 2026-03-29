# RainstormOS

A hobby x86-64 kernel written in C. Started as a 32-bit project, now fully 64-bit.

I'm building this because low-level systems programming is interesting to me,
and an OS is the best way to actually understand what's going on under the hood.


## What works right now

- Boots via Limine in 64-bit long mode
- Higher-half kernel with HHDM mapping
- Bitmap-based physical memory manager
- 4-level paging with per-section flags (RX/RO/RW)
- Framebuffer graphics
- Exception handling

## What I'm planning on adding soon

- Virtual memory manager
- Kernel heap (kmalloc/kfree)
- ACPI parsing
- APIC + keyboard driver
- TTY and basic kernel shell
- Timer
- Cooperative multitasking + context switching
- Proper synchronization primitives (spinlocks, mutexes)
- Preemptive kernel threads
- Usermode
- Syscall interface

## Later

- VFS layer (probably backed by a ramfs first)
- ATA disk driver
- AHCI
- ext2 or FAT32
- ELF loader
- Linux syscall compatibility layer

## Long shot goals

- Full Linux binary compatibility
- X11 + a basic window manager

## Notes

This is purely a learning project. I expect to refactor things heavily as I go
that's kind of the point. If you see something broken or have a suggestion, open an issue.