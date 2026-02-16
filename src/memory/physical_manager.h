#pragma once
#include "multiboot.h"
#include <stdbool.h>

#define PAGE_SIZE 4096
#define PAGES_PER_ROW 8

#define PMM_ALIGN_UP(x) ((((uint32_t)(x) + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE) // (x + page_size - 1 ) / page_size * page_size
#define PMM_ALIGN_DOWN(x) (((uint32_t)(x) / PAGE_SIZE) * PAGE_SIZE)   // x / page_size * page_size

typedef enum pmm_memory_type
{
  PMM_FREE = 0,
  PMM_ALLOCATED = 1
} pmm_memory_type;

typedef uint8_t* phys_addr_t;

void pmm_init(multiboot_info *mb_info, uint8_t *kernel_start, uint8_t *kernel_end);

phys_addr_t pmm_alloc_page();
void pmm_free_page(phys_addr_t phys_addr);

bool pmm_mark_reserved(phys_addr_t phys_addr);

bool pmm_check_alignment(phys_addr_t phys_addr);
bool pmm_is_region_free(phys_addr_t phys_addr);

uint32_t pmm_get_free_size();
uint32_t pmm_get_total_size();