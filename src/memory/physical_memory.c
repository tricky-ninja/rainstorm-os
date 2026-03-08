/**
 *  PMM Implementation
 *
 *  Uses bitmap allocator:
 *      1 bit = 1 physical page (4KB)
 *
 *  Reserved:
 *      - First 1MB
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
static size_t bitmap_pagecount;

static phys_addr_t kernel_start_physical;
static phys_addr_t kernel_end_physical;

static phys_addr_t bitmap_start_addr_physical;
static phys_addr_t bitmap_end_addr_physical;

bool initialized = false;

static phys_addr_t page_index_to_addr(size_t page_index)
{
  return (page_index * PAGE_SIZE);
}

static size_t page_index_from_byte_and_bit(size_t byte_index, uint8_t bit_index)
{
  return (byte_index * BITS_PER_BYTE) + bit_index;
}

static size_t addr_to_page_index(phys_addr_t addr)
{
  addr = PMM_ALIGN_DOWN(addr);
  return (addr / PAGE_SIZE);
}

static size_t bitmap_size_in_bytes(size_t page_index)
{
  return ((page_index + BITS_PER_BYTE - 1) / BITS_PER_BYTE);
}

static int mark_page(size_t page_index, pmm_memory_type type)
{
  if (page_index >= bitmap_pagecount)
    return -1;
  size_t byte_index = BYTE_INDEX(page_index);
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

static int clip_range(pmm_range_t *range, phys_addr_t limit)
{
  if (range->start >= limit)
    return -1;
  if (range->end > limit)
    range->end = limit;
  if (range->end <= range->start)
    return -1;

  return 0;
}

void pmm_init(pmm_range_t *mem_ranges, size_t mem_ranges_count, uint8_t *bitmap_start_addr, phys_addr_t bitmap_start_physical, phys_addr_t kernel_start_addr_physical, phys_addr_t kernel_end_addr_physical)
{
  if (!pmm_is_aligned(bitmap_start_physical))
  {
    klog_critical("pmm_init bitmap_start_physical is not page alligned\n");
    return;
  }

  bitmap_start = bitmap_start_addr;
  bitmap_start_addr_physical = bitmap_start_physical;
  bitmap_pagecount = 0;

  // Initialize bitmap and mark every page as used
  {
    phys_addr_t highest_memory = 0;
    for (size_t i = 0; i < mem_ranges_count; i++)
    {
      pmm_range_t range = mem_ranges[i];
      if (clip_range(&range, PMM_LIMIT_32) != 0)
        continue;
      highest_memory = highest_memory > range.end ? highest_memory : range.end;
    }
    bitmap_pagecount = PMM_ALIGN_UP(highest_memory) / PAGE_SIZE;
    bitmap_end = bitmap_start + bitmap_size_in_bytes(bitmap_pagecount);
    bitmap_end_addr_physical = bitmap_start_addr_physical + bitmap_size_in_bytes(bitmap_pagecount);
    memset(bitmap_start, 0xFF, bitmap_size_in_bytes(bitmap_pagecount));
  }
  
  // Mark free ranges as free
  bool found = false;
  for (size_t i = 0; i < mem_ranges_count; i++)
  {
    pmm_range_t range = mem_ranges[i];
    if (clip_range(&range, PMM_LIMIT_32) != 0)
      continue;

    if (range.type != PMM_FREE)
      continue;

    phys_addr_t base_addr = PMM_ALIGN_UP(range.start);
    phys_addr_t end_addr = PMM_ALIGN_DOWN(range.end);

    if (end_addr <= base_addr)
      continue;

    size_t page_count = (end_addr - base_addr) / PAGE_SIZE;
    size_t base_page_index = addr_to_page_index(base_addr);
    found = true;
    for (size_t count = 0; count < page_count; count++)
    {
      mark_page(base_page_index + count, PMM_FREE);
    }
  }
  if (!found)
  {
    klog_critical("pmm_init: No free pages found, ram is unusable");
    return;
  }

  // Mark allocated ranges as allocated
  for (size_t i = 0; i < mem_ranges_count; i++)
  {
    pmm_range_t range = mem_ranges[i];
    if (clip_range(&range, PMM_LIMIT_32) != 0)
      continue;

    if (range.type == PMM_FREE)
      continue;

    phys_addr_t base_addr = PMM_ALIGN_DOWN(range.start);
    phys_addr_t end_addr = PMM_ALIGN_UP(range.end);

    if (end_addr <= base_addr)
    {
      klog_critical("pmm_init: end_addr is less than base_addr, this was not supposed to happen base=0x%llx end=0x%llx (addr=0x%llx len=0x%llx type=%u)",
                    base_addr, end_addr, range.start, range.end, (unsigned)range.type);
      asm volatile("cli");
      asm volatile("hlt");
      __builtin_unreachable();
    }

    size_t page_count = (end_addr - base_addr) / PAGE_SIZE;
    size_t base_page_index = addr_to_page_index(base_addr);

    for (size_t count = 0; count < page_count; count++)
    {
      mark_page(base_page_index + count, PMM_ALLOCATED);
    }
  }

  // Mark kernel and bitmap memory as reserved
  {
    phys_addr_t start = PMM_ALIGN_DOWN(kernel_start_addr_physical);
    phys_addr_t end = PMM_ALIGN_UP(kernel_end_addr_physical);
    kernel_start_physical = start;
    kernel_end_physical = end;

    size_t start_index = addr_to_page_index(start);
    size_t end_index = addr_to_page_index(end);

    while (start_index < end_index)
    {
      mark_page(start_index, PMM_ALLOCATED);
      start_index += 1;
    }

    start = PMM_ALIGN_DOWN(bitmap_start_addr_physical);
    end = PMM_ALIGN_UP(bitmap_end_addr_physical);
    start_index = addr_to_page_index(start);
    end_index = addr_to_page_index(end);

    while (start_index < end_index)
    {
      mark_page(start_index, PMM_ALLOCATED);
      start_index += 1;
    }
  }

  // Mark the first 1MB memory as reserved
  {
    size_t page_count = PMM_ALIGN_UP(1024 * 1024) / PAGE_SIZE;
    for (size_t i = 0; i < page_count; i++)
      mark_page(i, PMM_ALLOCATED);
  }
  initialized = true;
  klog_info("pmm_init: Successfully intiliased physical memory");
}

phys_addr_t pmm_alloc_page()
{
  if (!initialized)
  {
    klog_error("pmm_alloc_page called before initialising bitmap");
    return PMM_INVALID_ADDRESS;
  }
  size_t bitmap_size = bitmap_size_in_bytes(bitmap_pagecount);
  
  for (size_t byte = 0; byte < bitmap_size; byte++)
  {
    if (bitmap_start[byte] == 0xFF) continue;
    for (uint8_t bit = 0; bit < BITS_PER_BYTE; bit++)
    {
      uint8_t *bitmap_byte = &bitmap_start[byte];
      uint8_t value = ((*bitmap_byte) >> bit) & 1u;
      if (value == PMM_ALLOCATED)
        continue;

      size_t page_index = page_index_from_byte_and_bit(byte, bit);
      if (page_index >= bitmap_pagecount) break;
      if (mark_page(page_index, PMM_ALLOCATED) != 0) continue;
      return page_index_to_addr(page_index);
    }
  }
  klog_critical("pmm_alloc_page: No usable memory found, ram might be filled up");
  return PMM_INVALID_ADDRESS;
}

void pmm_free_page(phys_addr_t phys_addr)
{
  if (!initialized)
  {
    klog_error("pmm_free_page called before initialising bitmap");
    return;
  }

  if (!pmm_is_aligned(phys_addr))
  {
    klog_error("pmm_free_page: 0x%llx is not page alligned", phys_addr);
    return;
  }

  if (phys_addr < 1024 * 1024)
  {
    klog_error("pmm_free_page: Cannot free reserved pages");
    return;
  }

  if (phys_addr >= kernel_start_physical && phys_addr < kernel_end_physical)
  {
    klog_error("pmm_free_page: Cannot free reserved pages");
    klog_debug("tried to free kernel pages");
    return;
  }

  if (phys_addr >= bitmap_start_addr_physical && phys_addr < bitmap_end_addr_physical)
  {
    klog_error("pmm_free_page: Cannot free reserved pages");
    klog_debug("tried to free bitmap pages");
    return;
  }

  size_t page_index = addr_to_page_index(phys_addr);
  if (page_index >= bitmap_pagecount)
  {
    klog_error("pmm_free_page: Address out of range");
    return;
  }

  if (pmm_is_page_free(phys_addr)) klog_warn("pmm_free_page: Attempted to free an already free page");

  mark_page(page_index, PMM_FREE);
}

bool pmm_is_aligned(phys_addr_t phys_addr)
{
  return (phys_addr % PAGE_SIZE) == 0;
}

bool pmm_is_page_free(phys_addr_t phys_addr)
{
  if (!initialized)
  {
    klog_error("pmm_is_page_free called before initialising bitmap");
    return false;
  }
  if (!pmm_is_aligned(phys_addr))
  {
    klog_error("pmm_is_page_free: 0x%llx is not page alligned", phys_addr);
    return false;
  }

  size_t page_index = addr_to_page_index(phys_addr);
  if (page_index >= bitmap_pagecount)
  {
    klog_error("pmm_is_page_free: Address out of range");
    return false;
  }
  size_t byte_index = BYTE_INDEX(page_index);
  uint8_t bit_index = BIT_INDEX(page_index);

  uint8_t *bitmap_byte = bitmap_start + byte_index;
  uint8_t value = ((*bitmap_byte) >> bit_index) & 1u;
  if (value == PMM_FREE)
    return true;
  return false;
}

size_t pmm_get_free_size()
{
  if (!initialized)
  {
    klog_error("pmm_get_free_size called before initialising bitmap");
    return 0;
  }
  size_t free_page_count = 0;
  for (size_t page_index = 0; page_index < bitmap_pagecount; page_index++)
  {

    size_t byte_index = BYTE_INDEX(page_index);
    uint8_t bit_index = BIT_INDEX(page_index);

    uint8_t *bitmap_byte = bitmap_start + byte_index;
    uint8_t value = ((*bitmap_byte) >> bit_index) & 1u;
    if (value == PMM_FREE)
      free_page_count++;
  }
  return free_page_count * PAGE_SIZE;
}

size_t pmm_get_total_size()
{
  if (!initialized)
  {
    klog_error("pmm_get_total_size called before initialising bitmap");
    return 0;
  }
  return bitmap_pagecount * PAGE_SIZE;
}

int pmm_mark_allocated(phys_addr_t phys_addr)
{
  if (!initialized)
  {
    klog_error("pmm_mark_allocated called before initialising bitmap");
    return -1;
  }
  if (!pmm_is_aligned(phys_addr))
  {
    klog_error("pmm_mark_allocated: Address 0x%llx is not page aligned\n", phys_addr);
    return -1;
  }
  size_t page_index = addr_to_page_index(phys_addr);
  return mark_page(page_index, PMM_ALLOCATED);
}