set architecture i386
set disassembly-flavor intel

file build/Rainstorm/boot/kernel
target remote localhost:1234
b _start

define paging
    monitor info mem
end

define tlb
    monitor info tlb
end

define regs
    monitor info registers
end

define physx
    monitor xp /10wx $arg0
end