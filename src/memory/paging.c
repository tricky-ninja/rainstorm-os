#include "paging.h"
#include "klog.h"
#include "memory_utils.h"
#include "arch/i686/isr.h"

#define KERNEL_VMA 0xC0000000
#define PAGE_DIR_VIRT_ADDR ((page_dir_entry *)0xFFFFF000)
#define PAGE_TABLE_VIRT_ADDR(i) ((uint32_t *)(0xFFC00000 + (i) * PAGE_SIZE))

static bool initialized = false;

// Internal assembly helpers (defined in paging.asm)
void _paging_switch_dir(phys_addr_t dir_addr);
void _paging_flush_tlb();
void _paging_enable();
void _paging_disable_pse();
bool _paging_is_enabled();

void page_fault_handler(Registers *regs)
{
    (void)regs;
    klog_critical("Page fault");
}

phys_addr_t paging_init(phys_addr_t start, phys_addr_t end)
{
    if (!_paging_is_enabled())
    {
        klog_critical("paging_init: Called without setting up intial 4MB paging");  // for now it panics and doesnt return
        return PMM_INVALID_ADDRESS;
    }

    if (!pmm_is_aligned(start) || !pmm_is_aligned(end))
    {
        klog_error("paging_init: Address are not page aligned, start=0x%x end=0x%x", start, end);
        return PMM_INVALID_ADDRESS;
    }

    if ((uint64_t)((uint64_t)end + KERNEL_VMA) >= PMM_LIMIT_32)
    {
        klog_warn("paging_init: end=0x%x is higher than 32 bit integer limit when accounting for higher half\n", end);
       end = 0x3FFFFFFF;    // end + KERNEL_VMA = 32 bit integer limit 
    }

    phys_addr_t kernel_page_dir;
    kernel_page_dir = pmm_alloc_page();
    page_dir_entry *kernel_page_dir_va = (page_dir_entry*)(kernel_page_dir + KERNEL_VMA);
    memset(kernel_page_dir_va, 0, sizeof(page_dir_entry) * 1024);

    for (phys_addr_t addr = start; addr < end; addr += PAGE_SIZE)
    {
        uintptr_t virt_addr = addr + KERNEL_VMA;

        uint32_t dir_index = DIR_INDEX(virt_addr);
        uint32_t table_index = TABLE_INDEX(virt_addr);

        
        if (!(kernel_page_dir_va[dir_index] & PAGE_FLAG_PRESENT))
        {
            phys_addr_t new_page_table_addr = pmm_alloc_page();
            memset((void*)(new_page_table_addr + KERNEL_VMA), 0, PAGE_SIZE);
            kernel_page_dir_va[dir_index] = new_page_table_addr | PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE;
        }

        phys_addr_t table_phys_addr = kernel_page_dir_va[dir_index] & ~(0xFFFU);

        uint32_t *table_virt_addr = (uint32_t*)(table_phys_addr + KERNEL_VMA);
        table_virt_addr[table_index] = addr | PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE;
    }

    kernel_page_dir_va[1023] = kernel_page_dir | PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE;

    paging_switch_dir(kernel_page_dir);
    _paging_disable_pse();
    initialized = true;
    isr_register_handler(14, page_fault_handler);
    return kernel_page_dir;
}

void paging_switch_dir(phys_addr_t dir)
{
    _paging_switch_dir(dir);
    return;
}

phys_addr_t paging_get_current_dir()
{
    if (!initialized)
    {
        klog_error("paging_map_page: Called before initialising paging");
        return PMM_INVALID_ADDRESS;
    }
    return PAGE_DIR_VIRT_ADDR[1023] & ~0xFFFU;
}

/*
TODO: only propagate USER flag to PDE, nothing else
uint32_t pde_flags = PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE;
if (flags & PAGE_FLAG_USER) pde_flags |= PAGE_FLAG_USER;
dir_entries[dir_index] = new_page_table_addr | pde_flags;
*/

void paging_map_page(uintptr_t virt_addr, phys_addr_t phys_addr, uint32_t flags)
{
    if (!initialized)
    {
        klog_error("paging_map_page: Called before initialising paging");
        return;
    }
    if (!pmm_is_aligned(virt_addr) || !pmm_is_aligned(phys_addr))
    {
        klog_error("paging_map_page: Address are not page aligned, virt_addr=0x%x phys_addr=0x%x", virt_addr, phys_addr);
        return;
    }
    page_dir_entry *dir_entries = PAGE_DIR_VIRT_ADDR;
    
    uint32_t dir_index = DIR_INDEX(virt_addr);
    uint32_t table_index = TABLE_INDEX(virt_addr);

    if (!(dir_entries[dir_index] & PAGE_FLAG_PRESENT))
    {
        phys_addr_t new_page_table_addr = pmm_alloc_page();
        dir_entries[dir_index] = new_page_table_addr | flags | PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE;
        uint32_t *page_table = PAGE_TABLE_VIRT_ADDR(dir_index);
        memset((void*)page_table, 0, PAGE_SIZE);
    }

    uint32_t *page_table = PAGE_TABLE_VIRT_ADDR(dir_index);
    page_table[table_index] = phys_addr | flags | PAGE_FLAG_PRESENT;
    _paging_flush_tlb();
}

bool paging_is_mapped(uintptr_t virt_addr)
{
    if (!initialized)
    {
        klog_error("paging_is_mapped: Called before initializing paging");
        return false;
    }


    page_dir_entry *page_dir = PAGE_DIR_VIRT_ADDR;
    uint32_t dir_index = DIR_INDEX(virt_addr);
    uint32_t table_index = TABLE_INDEX(virt_addr);

    if (!(page_dir[dir_index] & PAGE_FLAG_PRESENT)) return false;
    uint32_t *page_table = PAGE_TABLE_VIRT_ADDR(dir_index);
    if (!(page_table[table_index] & PAGE_FLAG_PRESENT)) return false;
    return true;

}

void paging_unmap_page(uintptr_t virt_addr)
{
    if (!initialized)
    {
        klog_error("paging_umap_page: Called before initialising paging");
        return;
    }
    if (!pmm_is_aligned(virt_addr))
    {
        klog_error("paging_umap_page: Address are not page aligned, virt_addr=0x%x phys_addr=0x%x", virt_addr);
        return;
    }

    page_dir_entry *page_dir = PAGE_DIR_VIRT_ADDR;
    uint32_t dir_index = DIR_INDEX(virt_addr);
    uint32_t table_index = TABLE_INDEX(virt_addr);

    if (!(page_dir[dir_index] & PAGE_FLAG_PRESENT)) return;
    uint32_t *page_table = PAGE_TABLE_VIRT_ADDR(dir_index);
    if (!(page_table[table_index] & PAGE_FLAG_PRESENT)) return;
    
    page_table[table_index] = 0;

}

// TODO: needs some work
phys_addr_t paging_create_dir()
{
    return pmm_alloc_page();
}

phys_addr_t paging_get_physical(uintptr_t virt_addr)
{
    page_dir_entry *page_dir = PAGE_DIR_VIRT_ADDR;
    uint32_t dir_index = DIR_INDEX(virt_addr);
    uint32_t table_index = TABLE_INDEX(virt_addr);

    if (!(page_dir[dir_index] & PAGE_FLAG_PRESENT)) return PMM_INVALID_ADDRESS;
    uint32_t *page_table = PAGE_TABLE_VIRT_ADDR(dir_index);
    if (!(page_table[table_index] & PAGE_FLAG_PRESENT)) return PMM_INVALID_ADDRESS;
    
    return (phys_addr_t)((page_dir[dir_index] & 0xFFF) + (page_table[table_index] & 0xFFF) + PAGE_INDEX(virt_addr));
}