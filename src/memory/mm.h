#pragma once

#define PAGE_SIZE 0x1000ULL
#define PAGE_MASK (PAGE_SIZE - 1)

#define PAGE_ALIGN_DOWN(addr)  ((addr) & ~PAGE_MASK)
#define PAGE_ALIGN_UP(addr)    (((addr) + PAGE_MASK) & ~PAGE_MASK)
#define PAGE_COUNT(size)       (PAGE_ALIGN_UP(size) / PAGE_SIZE)
#define IS_PAGE_ALIGNED(addr)  (((addr) & PAGE_MASK) == 0)

extern uint64_t g_hhdm_offset;

#define HHDM_PHYS_TO_VIRT(phys) ((phys) + g_hhdm_offset)
#define HHDM_VIRT_TO_PHYS(virt) ((virt) - g_hhdm_offset)