#pragma once

#include <stdint.h>

#define MULTIBOOT_HEADER_MAGIC 0x1BADB002
#define MULTIBOOT_BOOTLOADER_MAGIC 0x2BADB002

#define MULTIBOOT_MEMORY_AVAILABLE 1
#define MULTIBOOT_MEMORY_RESERVED 2
#define MULTIBOOT_MEMORY_ACPI_RECLAIMABLE 3
#define MULTIBOOT_MEMORY_NVS 4
#define MULTIBOOT_MEMORY_BADRAM 5


// PACK this struct if any field is not uint32_t
// (https://cgit.git.savannah.gnu.org/cgit/grub.git/tree/doc/multiboot.h?h=multiboot)
typedef struct multiboot_info
{
  uint32_t flags;

  uint32_t mem_lower;
  uint32_t mem_upper;

  uint32_t boot_device;

  uint32_t cmdline;

  uint32_t mods_count; // module count
  uint32_t mods_addr;

  uint32_t symbols[4];

  // Memory map length and address
  uint32_t mmap_length;
  uint32_t mmap_addr;

  // (Unused for now)
  uint32_t drives_length;
  uint32_t drives_addr;

  /* ROM configuration table (unused for now) */
  uint32_t config_table;

  /* Boot Loader Name */
  uint32_t boot_loader_name;

  // Extend later as needed

} multiboot_info;

typedef struct multiboot_mmap_entry
{
  uint32_t size;
  uint64_t addr;
  uint64_t len;
  uint32_t type;
}__attribute__((packed)) multiboot_mmap_entry;

