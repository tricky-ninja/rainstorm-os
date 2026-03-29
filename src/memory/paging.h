#pragma once

#include <stdint.h>
#include "mm.h"
#include "pmm.h"
#include "utils/memory.h"

#define PAGE_PRESENT       ((uint64_t)(1ULL << 0))
#define PAGE_WRITABLE      ((uint64_t)(1ULL << 1))
#define PAGE_USER          ((uint64_t)(1ULL << 2))
#define PAGE_WRITE_THROUGH ((uint64_t)(1ULL << 3))
#define PAGE_CACHE_DISABLE ((uint64_t)(1ULL << 4))
#define PAGE_GLOBAL        ((uint64_t)(1ULL << 8))
#define PAGE_NO_EXECUTE    ((uint64_t)(1ULL << 63))

#define PML4_INDEX(vaddr) (((vaddr) >> 39) & 0x1FF)
#define PDPT_INDEX(vaddr) (((vaddr) >> 30) & 0x1FF)
#define PD_INDEX(vaddr)   (((vaddr) >> 21) & 0x1FF)
#define PT_INDEX(vaddr)   (((vaddr) >> 12) & 0x1FF)
#define PAGE_OFFSET(vaddr) ((vaddr) & 0xFFF)

#define PHYS_ADDR_MASK (0x000FFFFFFFFFF000ULL)

typedef struct kernel_segment_info
{
    uint8_t *limine_start;
    uint8_t *limine_end;
    uint8_t *text_start;
    uint8_t *text_end;
    uint8_t *rodata_start;
    uint8_t *rodata_end;
    uint8_t *data_start;
    uint8_t *data_end;
} kernel_segment_info_t;

phys_addr_t paging_init(phys_addr_t kernel_physical_base, uint64_t kernel_virtual_base, kernel_segment_info_t segment_info);

void paging_switch_pml4(phys_addr_t pml4_phys);

phys_addr_t paging_create_pml4();
void paging_destroy_pml4(phys_addr_t pml4_phys);

void paging_map(phys_addr_t pml4_phys,uint64_t virt_addr, phys_addr_t phys_addr, uint64_t flags);
void paging_map_range(phys_addr_t pml4_phys, uint64_t virt_addr, phys_addr_t phys_addr, size_t size, uint64_t flags);

void paging_unmap(phys_addr_t pml4_phys ,uint64_t virt_addr);
void paging_unmap_range(phys_addr_t pml4_phys, uint64_t virt_addr, size_t size);

bool paging_is_mapped(phys_addr_t pml4_phys, uint64_t virt_addr);
phys_addr_t paging_get_physical(phys_addr_t pml4_phys, uint64_t virt_addr);

phys_addr_t paging_get_current_pml4();