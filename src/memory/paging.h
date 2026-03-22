/*
    Operates under the assumption that page size is 4KB
*/

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "physical_memory.h"

#define PAGE_FLAG_PRESENT 0b1
#define PAGE_FLAG_WRITE 0b10
#define PAGE_FLAG_USER 0b100

#define DIR_INDEX(addr)   ((uintptr_t)(addr) >> 22)
#define TABLE_INDEX(addr) (((uintptr_t)(addr) >> 12) & 0x3ff)
#define PAGE_INDEX(addr)  ((uintptr_t)(addr) & 0xFFF)

typedef uint32_t page_dir_entry;

typedef struct page_dir_t
{
    phys_addr_t phys_addr;
    page_dir_entry *virt_addr;
} page_dir_t;

phys_addr_t paging_init(phys_addr_t start, phys_addr_t end);

void paging_switch_dir(phys_addr_t dir);
phys_addr_t paging_get_current_dir();

phys_addr_t paging_create_dir();
void paging_destroy_dir(phys_addr_t dir);

void paging_map_page(uintptr_t virt_addr, phys_addr_t phys_addr, uint32_t flags);
void paging_unmap_page(uintptr_t virt_addr);

bool paging_is_mapped(uintptr_t virt_addr);
phys_addr_t paging_get_physical(uintptr_t virt_addr);


