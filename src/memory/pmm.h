#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "limine.h"

typedef uint64_t phys_addr_t;

#define PMM_INVALID_ADDRESS ((phys_addr_t)-1ULL)

typedef enum pmm_memory_type
{
    PMM_FREE = 0,
    PMM_ALLOCATED = 1
} pmm_memory_type;


void pmm_init(struct limine_memmap_response *memmap, uint64_t hhdm_offset);

phys_addr_t pmm_alloc_frame();
phys_addr_t pmm_alloc_frames(size_t count);

void pmm_free_frame(phys_addr_t phys_addr);
void pmm_free_frames(phys_addr_t phys_addr, size_t count);

void pmm_mark_allocated(phys_addr_t phys_addr);

bool pmm_is_frame_free(phys_addr_t phys_addr);

size_t pmm_get_free_size();
size_t pmm_get_total_size();