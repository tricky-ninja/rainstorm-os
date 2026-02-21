/**
 *  PMM Implementation
 *
 *  Uses bitmap allocator:
 *      1 bit = 1 physical page (4KB)
 *
 *  Reserved:
 *      - Page 0
 *      - Kernel region
 *      - Non-available multiboot regions
 *
 *  Allocation strategy: First-fit.
 *  Author: Sreyas A (TrickyNinja)
 */

#include "physical_memory.h"
#include <stddef.h>
#include "memory_utils.h"
#include "klog.h"

#define BIT_INDEX(x) ((x) % BITS_PER_BYTE)
#define BYTE_INDEX(x) ((x) / BITS_PER_BYTE)

static uint8_t *bitmap_start;
static uint8_t *bitmap_end;
static uint32_t bitmap_pagecount;

bool intialised = false;

phys_addr_t page_index_to_addr(uint32_t page_index)
{
  return (page_index * PAGE_SIZE);
}

uint32_t page_index_from_byte_and_bit(uint32_t byte_index, uint32_t bit_index)
{
  return (byte_index * BITS_PER_BYTE) + bit_index;
}

uint32_t addr_to_page_index(phys_addr_t addr)
{
  addr = PMM_ALIGN_DOWN(addr);
  return ((uint32_t)addr / PAGE_SIZE);
}

uint32_t bitmap_size_in_bytes(uint32_t page_index)
{
  return ((page_index + BITS_PER_BYTE - 1) / BITS_PER_BYTE);
}

int mark_page(uint32_t page_index, pmm_memory_type type)
{
  if (page_index >= bitmap_pagecount)
    return false;
  uint32_t byte_index = BYTE_INDEX(page_index);
  uint8_t bit_index = BIT_INDEX(page_index);

  uint8_t *bitmap_byte = bitmap_start + byte_index;
  uint8_t mask = (uint8_t)(1u << bit_index);
  if (type == PMM_FREE)
  {
    *bitmap_byte &= ~mask;
    return 0;
  }

  if (type == PMM_ALLOCATED)
  {
    *bitmap_byte |= mask;
    return 0;
  }
  return -1;
}

void pmm_init(multiboot_info *mb_info, phys_addr_t kernel_start_addr, phys_addr_t kernel_end_addr)
{
  bitmap_start = PMM_ALIGN_UP(kernel_end_addr);
  bitmap_pagecount = 0;

  // Initialise bitmap size and mark every page as used
  {
    uint8_t *ptr = (uint8_t *)mb_info->mmap_addr;
    uint8_t *end = ptr + mb_info->mmap_length;
    phys_addr_t highest_usable_memory = 0;
    while (ptr < end)
    {
      multiboot_mmap_entry *entry = (multiboot_mmap_entry *)ptr;

      // Return if entry is above 4GB
      if (entry->addr > 0xFFFFFFFF)
      {
        ptr += entry->size + sizeof(entry->size);
        continue;
      }

      uint32_t mem_len = (uint32_t)entry->len;
      if ((entry->addr + entry->len) > 0xFFFFFFFF)
        mem_len = (uint32_t)(0xFFFFFFFF - entry->addr);

      if (entry->type == MULTIBOOT_MEMORY_AVAILABLE)
        highest_usable_memory = highest_usable_memory > (entry->addr + mem_len) ? highest_usable_memory : (entry->addr + mem_len);
      ptr += entry->size + sizeof(entry->size);
    }

    bitmap_pagecount = PMM_ALIGN_UP(highest_usable_memory) / PAGE_SIZE;
    memset(bitmap_start, 0xFF, bitmap_size_in_bytes(bitmap_pagecount));
    bitmap_end = bitmap_start + bitmap_size_in_bytes(bitmap_pagecount);
  }

  // Loop through memory map and mark available ram as free
  {
    uint8_t *ptr = (uint8_t *)mb_info->mmap_addr;
    uint8_t *end = ptr + mb_info->mmap_length;
    while (ptr < end)
    {
      multiboot_mmap_entry *entry = (multiboot_mmap_entry *)ptr;

      // Return if entry is above 4GB
      if (entry->addr > 0xFFFFFFFF)
      {
        ptr += entry->size + sizeof(entry->size);
        continue;
      }

      uint32_t mem_len = (uint32_t)entry->len;
      if ((entry->addr + entry->len) > 0xFFFFFFFF)
        mem_len = (uint32_t)(0xFFFFFFFF - entry->addr);

      if (entry->type != MULTIBOOT_MEMORY_AVAILABLE)
      {
        ptr += entry->size + sizeof(entry->size);
        continue;
      }

      phys_addr_t base_addr = PMM_ALIGN_DOWN(entry->addr);
      phys_addr_t end_addr = PMM_ALIGN_UP(entry->addr + mem_len);
      uint32_t page_count = (end_addr - base_addr) / PAGE_SIZE;
      uint32_t page_base_index = addr_to_page_index(base_addr);
      for (uint32_t i = 0; i < page_count; i++)
      {
        mark_page(page_base_index + i, PMM_FREE);
      }

      ptr += entry->size + sizeof(entry->size);
    }
  }

  // Loop through memory map and mark reserved ram as allocated
  {
    uint8_t *ptr = (uint8_t *)mb_info->mmap_addr;
    uint8_t *end = ptr + mb_info->mmap_length;
    while (ptr < end)
    {
      multiboot_mmap_entry *entry = (multiboot_mmap_entry *)ptr;

      // Return if entry is above 4GB
      if (entry->addr > 0xFFFFFFFF)
      {
        ptr += entry->size + sizeof(entry->size);
        continue;
      }

      uint32_t mem_len = (uint32_t)entry->len;
      if ((entry->addr + entry->len) > 0xFFFFFFFF)
        mem_len = (uint32_t)(0xFFFFFFFF - entry->addr);

      if (entry->type == MULTIBOOT_MEMORY_AVAILABLE)
      {
        ptr += entry->size + sizeof(entry->size);
        continue;
      }

      phys_addr_t base_addr = PMM_ALIGN_DOWN(entry->addr);
      phys_addr_t end_addr = PMM_ALIGN_UP(entry->addr + mem_len);
      uint32_t page_count = (end_addr - base_addr) / PAGE_SIZE;
      uint32_t page_base_index = addr_to_page_index(base_addr);
      for (uint32_t i = 0; i < page_count; i++)
      {
        mark_page(page_base_index + i, PMM_ALLOCATED);
      }

      ptr += entry->size + sizeof(entry->size);
    }
  }

  // Mark kernel and bitmap memory as reserved
  {
    phys_addr_t start = kernel_start_addr;
    phys_addr_t end = PMM_ALIGN_UP(bitmap_end);
    uint32_t start_index = addr_to_page_index(start);
    uint32_t end_index = addr_to_page_index(end);

    while (start_index < end_index)
    {
      mark_page(start_index, PMM_ALLOCATED);
      start_index += 1;
    }
  }

  // Mark the first 1MB memory as reserved
  uint32_t page_count = PMM_ALIGN_UP(1024 * 1024) / PAGE_SIZE;
  for (uint32_t i = 0; i < page_count; i++)
    mark_page(i, PMM_ALLOCATED);

  intialised = true;
}

// TODO: use byte iteration method instead of bit iteration
phys_addr_t pmm_alloc_page()
{
  if (!intialised)
  {
    klog_error("pmm_alloc_page called before initialising bitmap");
    return PMM_INVALID_ADDRESS;
  }
  for (uint32_t page_index = 0; page_index < bitmap_pagecount; page_index++)
  {
    uint32_t byte_index = page_index / BITS_PER_BYTE;
    uint8_t bit_index = page_index % BITS_PER_BYTE;

    uint8_t *bitmap_byte = bitmap_start + byte_index;

    uint8_t value = ((*bitmap_byte) >> bit_index) & 1u;
    if (value == PMM_ALLOCATED)
      continue;

    mark_page(page_index, PMM_ALLOCATED);
    return page_index_to_addr(page_index);
  }
  return PMM_INVALID_ADDRESS;
}

void pmm_free_page(phys_addr_t phys_addr)
{
  if (!intialised)
  {
    klog_error("pmm_free_page called before initialising bitmap");
    return;
  }
  if (!pmm_is_aligned(phys_addr))
    return;

  uint32_t page_index = addr_to_page_index(phys_addr);
  if (page_index >= bitmap_pagecount)
    return;
  mark_page(page_index, PMM_FREE);
}

bool pmm_is_aligned(phys_addr_t phys_addr)
{
  return ((uint32_t)phys_addr % PAGE_SIZE) == 0;
}

bool pmm_is_page_free(phys_addr_t phys_addr)
{
  if (!intialised)
  {
    klog_error("pmm_is_page_free called before initialising bitmap");
    return;
  }
  if (!pmm_is_aligned(phys_addr))
    return false;

  uint32_t page_index = addr_to_page_index(phys_addr);
  if (page_index >= bitmap_pagecount)
    return false;
  uint32_t byte_index = BYTE_INDEX(page_index);
  uint8_t bit_index = BIT_INDEX(page_index);

  uint8_t *bitmap_byte = bitmap_start + byte_index;
  uint8_t value = ((*bitmap_byte) >> bit_index) & 1u;
  if (value == PMM_FREE)
    return true;
  return false;
}

uint32_t pmm_get_free_size()
{
  if (!intialised)
  {
    klog_error("pmm_get_free_size called before initialising bitmap");
    return 0;
  }
  uint32_t free_page_count = 0;
  for (uint32_t page_index = 0; page_index < bitmap_pagecount; page_index++)
  {

    uint32_t byte_index = BYTE_INDEX(page_index);
    uint8_t bit_index = BIT_INDEX(page_index);

    uint8_t *bitmap_byte = bitmap_start + byte_index;
    uint8_t value = ((*bitmap_byte) >> bit_index) & 1u;
    if (value == PMM_FREE)
      free_page_count++;
  }
  return free_page_count * PAGE_SIZE;
}

uint32_t pmm_get_total_size()
{
  if (!intialised)
  {
    klog_error("pmm_get_total_size called before initialising bitmap");
    return 0;
  }
  return bitmap_pagecount * PAGE_SIZE;
}

int pmm_mark_allocated(phys_addr_t phys_addr)
{
  if (!intialised)
  {
    klog_error("pmm_mark_allocated called before initialising bitmap");
    return -1;
  }
  uint32_t page_index = addr_to_page_index(phys_addr);
  return mark_page(page_index, PMM_ALLOCATED);
}