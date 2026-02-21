/**
 *  Module: Physical Memory Manager (PMM)
 *  Purpose: Page-based physical memory allocation
 *
 *  Provides:
 *      - Allocate/free 4KB pages
 *      - Memory statistics
 *      - Page state queries
 *
 *  Initialized using Multiboot memory map.
 *  Author: Sreyas A (TrickyNinja)
 */

#pragma once
#include "multiboot.h"
#include <stdbool.h>

#define PAGE_SIZE 4096
#define BITS_PER_BYTE 8

#define PMM_ALIGN_UP(x) ((((uint32_t)(x) + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE) // (x + page_size - 1 ) / page_size * page_size
#define PMM_ALIGN_DOWN(x) (((uint32_t)(x) / PAGE_SIZE) * PAGE_SIZE)                 // x / page_size * page_size

#define PMM_INVALID_ADDRESS 0xFFFFFFFF

typedef enum pmm_memory_type
{
  PMM_FREE = 0,
  PMM_ALLOCATED = 1
} pmm_memory_type;

typedef uint32_t phys_addr_t;

/**
 *  Initializes the Physical Memory Manager using the multiboot memory map.
 *  This function:
 *    - Parses the memory map
 *    - Builds the bitmap
 *    - Marks reserved regions
 *    - Reserves kernel memory
 *    - Reserves the bitmap itself
 *    - Reserves the first 1MB of memory
 *
 *  Must be called before any other PMM function.
 *
 *  @param mb_info Pointer to multiboot information structure
 *  @param kernel_start_addr Physical start address of the kernel
 *  @param kernel_end_addr Physical end address of the kernel
 **/
void pmm_init(multiboot_info *mb_info, phys_addr_t kernel_start_addr, phys_addr_t kernel_end_addr);

/**
 *  Allocates a single 4KB physical page.
 *
 *  Scans the bitmap for a free page, marks it allocated,
 *  and returns its physical address.
 *
 *  @return Physical address of allocated page
 *  @return PMM_INVALID_ADDRESS if no free pages are available
 **/
phys_addr_t pmm_alloc_page();

/**
 *  Frees a previously allocated physical page.
 *
 *  The address must be page-aligned and within managed range.
 *  If invalid, the function silently ignores the request.
 *
 *  @param phys_addr Physical address of the page to free
 **/
void pmm_free_page(phys_addr_t phys_addr);

/**
 *  Marks a physical page as allocated in the bitmap.
 *
 *  Used internally and for manual reservation of memory.
 *  The address must be page-aligned.
 *
 *  @param phys_addr Physical address of the page
 *  @return 0 if the page was successfully marked
 *  @return -1 if the page index is out of range
 **/
int pmm_mark_allocated(phys_addr_t phys_addr);

/**
 *  Checks whether a physical address is aligned to a page boundary.
 *
 *  @param phys_addr Physical address to test
 *  @return true if address is aligned to PAGE_SIZE
 *  @return false otherwise
 **/
bool pmm_is_aligned(phys_addr_t phys_addr);

/**
 *  Checks whether a physical page is currently free.
 *
 *  The address must be page-aligned.
 *  If the address is invalid or out of range, false is returned.
 *
 *  @param phys_addr Physical address of the page
 *  @return true if the page is free
 *  @return false if allocated or invalid
 **/
bool pmm_is_page_free(phys_addr_t phys_addr);

/**
 *  Returns the total free physical memory in bytes.
 *
 *  Iterates through the bitmap and counts free pages.
 *
 *  @return Total free memory size in bytes
 **/
uint32_t pmm_get_free_size();
uint32_t pmm_get_total_size();