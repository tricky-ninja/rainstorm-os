#include "shell/shell.h"
#include "drivers/x86/screen/vga.h"
#include "printf.h"
#include "memory/physical_memory.h"
#include "drivers/x86/keyboard/keyboard.h"

static shell_module_handler_return_t handler(char *args, multiboot_info *mb_info)
{
       (void)args;
       (void)mb_info;

       printf("========== PMM TEST BEGIN ==========\n");

       uint32_t total_pages = pmm_get_total_size() / PAGE_SIZE;
       uint32_t total_ram_mb = pmm_get_total_size() / 1024 / 1024;
       uint32_t free_before = pmm_get_free_size() / PAGE_SIZE;

       printf("Total pages: %u\n", total_pages);
       printf("Total RAM: %u MB\n", total_ram_mb);
       printf("Free pages (initial): %u\n", free_before);

       printf("\n[TEST] Alignment\n");

       printf("Aligned 0x1000: %u (expected 1)\n",
              pmm_is_aligned(0x1000));

       printf("Unaligned 0x1003: %u (expected 0)\n",
              pmm_is_aligned(0x1003));

       keyboard_get_char();
       printf("\n[TEST] Reserved regions\n");

       printf("First page free? %u (expected 0)\n",
              pmm_is_page_free(0));

       printf("Page at 0x1000 free? %u (expected 0)\n",
              pmm_is_page_free(0x1000));

       keyboard_get_char();
       printf("\n[TEST] Single allocation\n");

       phys_addr_t page = pmm_alloc_page();

       if (page == PMM_INVALID_ADDRESS)
       {
              printf("Allocation failed unexpectedly!\n");
       }
       else
       {
              printf("Allocated page: 0x%x\n", page);

              printf("Is free after alloc? %u (expected 0)\n",
                     pmm_is_page_free(page));
       }

       uint32_t free_after_alloc = pmm_get_free_size() / PAGE_SIZE;
       printf("Free pages after alloc: %u (expected %u)\n",
              free_after_alloc,
              free_before - 1);

       keyboard_get_char();
       printf("\n[TEST] Freeing page\n");

       pmm_free_page(page);

       printf("Is free after free? %u (expected 1)\n",
              pmm_is_page_free(page));

       uint32_t free_after_free = pmm_get_free_size() / PAGE_SIZE;
       printf("Free pages after free: %u (expected %u)\n",
              free_after_free,
              free_before);

       keyboard_get_char();
       printf("\n[TEST] Double free safety\n");

       pmm_free_page(page);
       printf("Double free did not crash (expected safe)\n");

       keyboard_get_char();
       printf("\n[TEST] pmm_mark_allocated()\n");

       phys_addr_t test_page = pmm_alloc_page();
       pmm_free_page(test_page);

       printf("Free before manual mark: %u (expected 1)\n",
              pmm_is_page_free(test_page));

       pmm_mark_allocated(test_page);

       printf("Free after manual mark: %u (expected 0)\n",
              pmm_is_page_free(test_page));

       pmm_free_page(test_page);

       keyboard_get_char();
       printf("\n[TEST] Exhaustion test\n");

       uint32_t alloc_count = 0;
       static phys_addr_t pages[1024];

       while (alloc_count < 1024)
       {
              phys_addr_t p = pmm_alloc_page();
              if (p == PMM_INVALID_ADDRESS)
                     break;

              pages[alloc_count++] = p;
       }

       printf("Allocated %u pages before stop\n", alloc_count);

       for (uint32_t i = 0; i < alloc_count; i++)
              pmm_free_page(pages[i]);

       printf("Free pages after restoring: %u (expected %u)\n",
              pmm_get_free_size() / PAGE_SIZE,
              free_before);
       keyboard_get_char();
       printf("\n[TEST] Invalid frees\n");

       pmm_free_page(12345); 
       printf("Unaligned free did not crash\n");

       pmm_free_page(0xFFFFFFFF); 
       printf("Out of bounds free did not crash\n");

       printf("\n[TEST] Final consistency\n");

       uint32_t free_final = pmm_get_free_size() / PAGE_SIZE;

       printf("Free pages final: %u (expected %u)\n",
              free_final,
              free_before);

       printf("========== PMM TEST END ==========\n");
       return 0;
}

SHELL_MODULE_REGISTER(
        .name = "memtest",
        .handler = handler,
        .desc = "Tests the physical memory manager");
