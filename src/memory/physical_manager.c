#include "physical_manager.h"
#include "printf.h"
#include "memory.h"

static uint8_t *bitmap_start;
static uint8_t *bitmap_end;
static uint32_t bitmap_pagecount;

phys_addr_t bit_to_addr(uint32_t bit_index)
{
  return (bit_index * PAGE_SIZE);
}

uint32_t get_bit_index(uint32_t row, uint32_t col)
{
  return (row * PAGES_PER_ROW) + col;
}

uint32_t addr_to_bit_index(phys_addr_t addr)
{
  addr = PMM_ALIGN_DOWN(addr);
  return ((uint32_t)addr / PAGE_SIZE);
}

uint32_t bit_to_byte(uint32_t bit_index)
{
  return ((bit_index + 7) / 8);
}

bool mark_page(uint32_t bit_index, pmm_memory_type type)
{
  if (bit_index >= bitmap_pagecount) return false;
  uint32_t row = bit_index / PAGES_PER_ROW;
  uint8_t col = bit_index % PAGES_PER_ROW;

  uint8_t *bitmap_row = bitmap_start + row;
  uint8_t mask = (uint8_t)(1u << col);
  if (type == PMM_FREE)
  {
    *bitmap_row &= ~mask;
    return true;
  }

  if (type == PMM_ALLOCATED)
  {
    *bitmap_row |= mask;
    return true;
  }
  return false;
}

void pmm_init(multiboot_info *mb_info, uint8_t *kernel_start, uint8_t *kernel_end)
{
  bitmap_start = PMM_ALIGN_UP((uint32_t)kernel_end + 0x1000);
  bitmap_pagecount = 0;

  // Initialise bitmap size and mark every page as used
  {
    uint8_t *ptr = (uint8_t *)mb_info->mmap_addr;
    uint8_t *end = ptr + mb_info->mmap_length;
    uint32_t highest_usable_memory = 0;
    while (ptr < end)
    {
      multiboot_mmap_entry *entry = (multiboot_mmap_entry *)ptr;
      if (entry->addr > 0xFFFFFFFF)
      {
        ptr += entry->size + sizeof(entry->size);
        continue;
      }

      uint32_t mem_len = (uint32_t)entry->len;
      if ((entry->addr + entry->len) > 0xFFFFFFFF)
        mem_len = (uint32_t)(0xFFFFFFFF - entry->addr);
      if (entry->type == MULTIBOOT_MEMORY_AVAILABLE) highest_usable_memory = highest_usable_memory > (entry->addr + mem_len) ? highest_usable_memory : (entry->addr + mem_len);
      ptr += entry->size + sizeof(entry->size);
    }
    bitmap_pagecount = PMM_ALIGN_UP(highest_usable_memory) / PAGE_SIZE;
    memset(bitmap_start, 0xFF, bit_to_byte(bitmap_pagecount));
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
      uint32_t page_count = PMM_ALIGN_UP(mem_len) / PAGE_SIZE;
      uint32_t page_base = addr_to_bit_index(base_addr);
      for (uint32_t i = 0; i < page_count; i++)
      {
        mark_page(page_base + i, PMM_FREE);
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
      uint32_t page_count = PMM_ALIGN_UP(mem_len) / PAGE_SIZE;
      uint32_t page_base = addr_to_bit_index(base_addr);
      for (uint32_t i = 0; i < page_count; i++)
      {
        mark_page(page_base + i, PMM_ALLOCATED);
      }

      ptr += entry->size + sizeof(entry->size);
    }
  }

  // Mark kernel and bitmap memory as reserved
  {
    phys_addr_t start = kernel_start;
    phys_addr_t end = bitmap_start + bit_to_byte(bitmap_pagecount);
    end = PMM_ALIGN_UP(end);
    uint32_t start_index = addr_to_bit_index(start);
    uint32_t end_index = addr_to_bit_index(end);

    while (start_index <= end_index)
    {
      mark_page(start_index, PMM_ALLOCATED);
      start_index += 1;
    }

  }

  // Mark the first 1MB memory as reserved
  uint32_t pages = (1024 * 1024) / PAGE_SIZE;
  memset(bitmap_start, 0xFF, bit_to_byte(pages)); 
  bitmap_end = bitmap_start + bit_to_byte(bitmap_pagecount);
}

phys_addr_t pmm_alloc_page()
{
  
}

void pmm_free_page(phys_addr_t phys_addr);

bool pmm_check_alignment(phys_addr_t phys_addr);
bool pmm_is_region_free(phys_addr_t phys_addr)
{
  // TODO: check allignment
  uint32_t bit_index = addr_to_bit_index(phys_addr);
  if (bit_index > bitmap_pagecount) return false;
  uint32_t row = bit_index / PAGES_PER_ROW;
  uint8_t col = bit_index % PAGES_PER_ROW;

  uint8_t *bitmap_row = bitmap_start + row;
  uint8_t value = ((*bitmap_row) >> col) & 1u;
  if (value == 0) return true;
  return false;
}

uint32_t pmm_get_free_size();
uint32_t pmm_get_total_size()
{
  return bitmap_pagecount * PAGE_SIZE;
}