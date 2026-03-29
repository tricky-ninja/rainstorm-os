#include "paging.h"
#include "utils/klog.h"

static phys_addr_t kernel_pml4_entries[256];

// Internal assembly helpers
void _paging_switch_dir(phys_addr_t pml4_phys);
void _paging_flush_tlb();
void _paging_invalidate_page(uint64_t virt_addr);
phys_addr_t _paging_get_current_dir();
void _paging_enable_nxe();

phys_addr_t paging_init(phys_addr_t kernel_physical_base, uint64_t kernel_virtual_base, kernel_segment_info_t segment_info)
{
    phys_addr_t pml4_phys = pmm_alloc_frame();
    uint64_t *pml4_entries = (uint64_t *)HHDM_PHYS_TO_VIRT(pml4_phys);
    memset((uint8_t *)HHDM_PHYS_TO_VIRT(pml4_phys), 0, PAGE_SIZE);

    // fill all kernel pml4 entries
    for (size_t i = 256; i < 512; i++)
    {
        phys_addr_t pdpt_phys = pmm_alloc_frame();
        uint64_t *pdpt_entries = (uint64_t *)HHDM_PHYS_TO_VIRT(pdpt_phys);
        memset(pdpt_entries, 0, PAGE_SIZE);
        pml4_entries[i] = pdpt_phys | PAGE_PRESENT | PAGE_WRITABLE;
    }

    // map hhdm
    size_t memsize = pmm_get_total_size();
    paging_map_range(pml4_phys, g_hhdm_offset, 0, memsize, PAGE_WRITABLE | PAGE_GLOBAL | PAGE_NO_EXECUTE | PAGE_PRESENT);

    // map kernel image
    size_t limine_size = (segment_info.limine_end - segment_info.limine_start);
    uint64_t limine_phys_addr = ((uint64_t)segment_info.limine_start - kernel_virtual_base) + kernel_physical_base;
    paging_map_range(pml4_phys, (uint64_t)segment_info.limine_start, limine_phys_addr, limine_size, PAGE_GLOBAL | PAGE_NO_EXECUTE | PAGE_PRESENT);

    size_t text_size = (segment_info.text_end - segment_info.text_start);
    uint64_t text_phys_addr = ((uint64_t)segment_info.text_start - kernel_virtual_base) + kernel_physical_base;
    paging_map_range(pml4_phys, (uint64_t)segment_info.text_start, text_phys_addr, text_size, PAGE_GLOBAL | PAGE_PRESENT);

    size_t rodata_size = (segment_info.rodata_end - segment_info.rodata_start);
    uint64_t rodata_phys_addr = ((uint64_t)segment_info.rodata_start - kernel_virtual_base) + kernel_physical_base;
    paging_map_range(pml4_phys, (uint64_t)segment_info.rodata_start, rodata_phys_addr, rodata_size, PAGE_GLOBAL | PAGE_NO_EXECUTE | PAGE_PRESENT);

    size_t data_size = (segment_info.data_end - segment_info.data_start);
    uint64_t data_phys_addr = ((uint64_t)segment_info.data_start - kernel_virtual_base) + kernel_physical_base;
    paging_map_range(pml4_phys, (uint64_t)segment_info.data_start, data_phys_addr, data_size, PAGE_GLOBAL | PAGE_NO_EXECUTE | PAGE_WRITABLE | PAGE_PRESENT);

    for (size_t i = 0; i < 256; i++)
        kernel_pml4_entries[i] = pml4_entries[i + 256];
    
    _paging_enable_nxe();
    _paging_switch_dir(pml4_phys);
    klog_debug("paging_init: paging initialised without crashing, pml4_phys=0x%p", pml4_phys);
    return pml4_phys;
}

void paging_switch_pml4(phys_addr_t pml4_phys)
{
    if (!IS_PAGE_ALIGNED(pml4_phys))
    {
        klog_error("paging_switch_pml4: pml4 address is not page alligned 0x%p", pml4_phys);
        return;
    }

    _paging_switch_dir(pml4_phys);
}

phys_addr_t paging_create_pml4()
{
    phys_addr_t pml4_phys = pmm_alloc_frame();
    uint64_t *pml4_entries = (uint64_t *)HHDM_PHYS_TO_VIRT(pml4_phys);
    for (size_t i = 0; i < 256; i++)
        pml4_entries[i] = 0;
    for (size_t i = 0; i < 256; i++)
        pml4_entries[i + 256] = kernel_pml4_entries[i];
    return pml4_phys;
}

void paging_destroy_pml4(phys_addr_t pml4_phys)
{
    if (!IS_PAGE_ALIGNED(pml4_phys))
    {
        klog_error("paging_switch_pml4: pml4 address is not page alligned 0x%p", pml4_phys);
        return;
    }

    uint64_t *pml4_entries = (uint64_t *)HHDM_PHYS_TO_VIRT(pml4_phys);
    for (size_t pml4_index = 0; pml4_index < 256; pml4_index++)
    {
        if (!(pml4_entries[pml4_index] & PAGE_PRESENT))
            continue;

        phys_addr_t pdpt_phys = pml4_entries[pml4_index] & PHYS_ADDR_MASK;
        uint64_t *pdpt_entries = (uint64_t *)HHDM_PHYS_TO_VIRT(pdpt_phys);
        for (size_t pdpt_index = 0; pdpt_index < 512; pdpt_index++)
        {
            if (!(pdpt_entries[pdpt_index] & PAGE_PRESENT))
                continue;

            phys_addr_t pd_phys = pdpt_entries[pdpt_index] & PHYS_ADDR_MASK;
            uint64_t *pd_entries = (uint64_t *)HHDM_PHYS_TO_VIRT(pd_phys);
            for (size_t pd_index = 0; pd_index < 512; pd_index++)
            {
                if (!(pd_entries[pd_index] & PAGE_PRESENT))
                    continue;

                phys_addr_t pt_phys = pd_entries[pd_index] & PHYS_ADDR_MASK;
                pmm_free_frame(pt_phys);
            }
            pmm_free_frame(pd_phys);
        }
        pmm_free_frame(pdpt_phys);
    }
    pmm_free_frame(pml4_phys);
}

void paging_map(phys_addr_t pml4_phys, uint64_t virt_addr, phys_addr_t phys_addr, uint64_t flags)
{
    if (!IS_PAGE_ALIGNED(pml4_phys))
    {
        klog_error("paging_map: pml4 address is not page alligned 0x%p", pml4_phys);
        return;
    }

    if (!IS_PAGE_ALIGNED(virt_addr))
    {
        klog_error("paging_map: virtual address is not page alligned 0x%p", virt_addr);
        return;
    }

    if (!IS_PAGE_ALIGNED(phys_addr))
    {
        klog_error("paging_map: physical address is not page alligned 0x%p", phys_addr);
        return;
    }

    uint64_t pml4_index = PML4_INDEX(virt_addr);
    uint64_t pdpt_index = PDPT_INDEX(virt_addr);
    uint64_t pd_index = PD_INDEX(virt_addr);
    uint64_t pt_index = PT_INDEX(virt_addr);

    uint64_t *pml4_entries = (uint64_t *)HHDM_PHYS_TO_VIRT(pml4_phys);
    uint64_t intermediate_flags = PAGE_PRESENT | PAGE_WRITABLE;
    if (flags & PAGE_USER)
        intermediate_flags |= PAGE_USER;

    if (!(pml4_entries[pml4_index] & PAGE_PRESENT))
    {
        phys_addr_t frame = pmm_alloc_frame();
        memset((uint8_t*)HHDM_PHYS_TO_VIRT(frame), 0, PAGE_SIZE);
        pml4_entries[pml4_index] = frame;
    }

    pml4_entries[pml4_index] |= intermediate_flags;
    phys_addr_t pdpt_phys = pml4_entries[pml4_index] & PHYS_ADDR_MASK;
    uint64_t *pdpt_entries = (uint64_t *)HHDM_PHYS_TO_VIRT(pdpt_phys);
    if (!(pdpt_entries[pdpt_index] & PAGE_PRESENT))
    {
        phys_addr_t frame = pmm_alloc_frame();
        memset((uint8_t*)HHDM_PHYS_TO_VIRT(frame), 0, PAGE_SIZE);
        pdpt_entries[pdpt_index] = frame;
    }
    pdpt_entries[pdpt_index] |= intermediate_flags;
    phys_addr_t pd_phys = pdpt_entries[pdpt_index] & PHYS_ADDR_MASK;
    uint64_t *pd_entries = (uint64_t *)HHDM_PHYS_TO_VIRT(pd_phys);
    if (!(pd_entries[pd_index] & PAGE_PRESENT))
    {
        phys_addr_t frame = pmm_alloc_frame();
        memset((uint8_t*)HHDM_PHYS_TO_VIRT(frame), 0, PAGE_SIZE);
        pd_entries[pd_index] = frame;
    }
    pd_entries[pd_index] |= intermediate_flags;
    phys_addr_t pt_phys = pd_entries[pd_index] & PHYS_ADDR_MASK;
    uint64_t *pt_entries = (uint64_t *)HHDM_PHYS_TO_VIRT(pt_phys);

    if (pt_entries[pt_index] & PAGE_PRESENT)
        klog_warn("paging_map: remapping already mapped address 0x%p from 0x%p to 0x%p", virt_addr, (pt_entries[pt_index] & PHYS_ADDR_MASK), phys_addr);

    pt_entries[pt_index] = phys_addr | flags;
    _paging_invalidate_page(virt_addr);
}

void paging_map_range(phys_addr_t pml4_phys, uint64_t virt_addr, phys_addr_t phys_addr, size_t size, uint64_t flags)
{
    if (!IS_PAGE_ALIGNED(pml4_phys))
    {
        klog_error("paging_map_range: pml4 address is not page alligned 0x%p", pml4_phys);
        return;
    }

    if (!IS_PAGE_ALIGNED(virt_addr))
    {
        klog_error("paging_map_range: virtual address is not page alligned 0x%p", virt_addr);
        return;
    }

    if (!IS_PAGE_ALIGNED(phys_addr))
    {
        klog_error("paging_map_range: physical address is not page alligned 0x%p", phys_addr);
        return;
    }

    size_t page_count = PAGE_COUNT(size);

    for (size_t i = 0; i < page_count; i++)
    {
        paging_map(pml4_phys, virt_addr + (i * PAGE_SIZE), phys_addr + (i * PAGE_SIZE), flags);
    }
}

void paging_unmap(phys_addr_t pml4_phys, uint64_t virt_addr)
{
    if (!IS_PAGE_ALIGNED(pml4_phys))
    {
        klog_error("paging_unmap: pml4 address is not page alligned 0x%p", pml4_phys);
        return;
    }

    if (!IS_PAGE_ALIGNED(virt_addr))
    {
        klog_error("paging_unmap: virtual address is not page alligned 0x%p", virt_addr);
        return;
    }

    uint64_t pml4_index = PML4_INDEX(virt_addr);
    uint64_t pdpt_index = PDPT_INDEX(virt_addr);
    uint64_t pd_index = PD_INDEX(virt_addr);
    uint64_t pt_index = PT_INDEX(virt_addr);

    uint64_t *pml4_entries = (uint64_t *)HHDM_PHYS_TO_VIRT(pml4_phys);
    if (!(pml4_entries[pml4_index] & PAGE_PRESENT))
    {
        klog_warn("paging_unmap: address 0x%p is not mapped", virt_addr);
        return;
    }

    phys_addr_t pdpt_phys = pml4_entries[pml4_index] & PHYS_ADDR_MASK;
    uint64_t *pdpt_entries = (uint64_t *)HHDM_PHYS_TO_VIRT(pdpt_phys);
    if (!(pdpt_entries[pdpt_index] & PAGE_PRESENT))
    {
        klog_warn("paging_unmap: address 0x%p is not mapped", virt_addr);
        return;
    }

    phys_addr_t pd_phys = pdpt_entries[pdpt_index] & PHYS_ADDR_MASK;
    uint64_t *pd_entries = (uint64_t *)HHDM_PHYS_TO_VIRT(pd_phys);
    if (!(pd_entries[pd_index] & PAGE_PRESENT))
    {
        klog_warn("paging_unmap: address 0x%p is not mapped", virt_addr);
        return;
    }

    phys_addr_t pt_phys = pd_entries[pd_index] & PHYS_ADDR_MASK;
    uint64_t *pt_entries = (uint64_t *)HHDM_PHYS_TO_VIRT(pt_phys);
    if (!(pt_entries[pt_index] & PAGE_PRESENT))
    {
        klog_warn("paging_unmap: address 0x%p is not mapped", virt_addr);
        return;
    }
    pt_entries[pt_index] = 0;
    _paging_invalidate_page(virt_addr);
}

void paging_unmap_range(phys_addr_t pml4_phys, uint64_t virt_addr, size_t size)
{
    if (!IS_PAGE_ALIGNED(pml4_phys))
    {
        klog_error("paging_unmap_range: pml4 address is not page alligned 0x%p", pml4_phys);
        return;
    }

    if (!IS_PAGE_ALIGNED(virt_addr))
    {
        klog_error("paging_unmap_range: virtual address is not page alligned 0x%p", virt_addr);
        return;
    }

    size_t page_count = PAGE_COUNT(size);

    for (size_t i = 0; i < page_count; i++)
    {
        paging_unmap(pml4_phys, virt_addr + (i * PAGE_SIZE));
    }
}

bool paging_is_mapped(phys_addr_t pml4_phys, uint64_t virt_addr)
{
    if (!IS_PAGE_ALIGNED(pml4_phys))
    {
        klog_error("paging_is_mapped: pml4 address is not page alligned 0x%p", pml4_phys);
        return false;
    }

    uint64_t pml4_index = PML4_INDEX(virt_addr);
    uint64_t pdpt_index = PDPT_INDEX(virt_addr);
    uint64_t pd_index = PD_INDEX(virt_addr);
    uint64_t pt_index = PT_INDEX(virt_addr);

    uint64_t *pml4_entries = (uint64_t *)HHDM_PHYS_TO_VIRT(pml4_phys);
    if (!(pml4_entries[pml4_index] & PAGE_PRESENT))
        return false;

    phys_addr_t pdpt_phys = pml4_entries[pml4_index] & PHYS_ADDR_MASK;
    uint64_t *pdpt_entries = (uint64_t *)HHDM_PHYS_TO_VIRT(pdpt_phys);
    if (!(pdpt_entries[pdpt_index] & PAGE_PRESENT))
        return false;

    phys_addr_t pd_phys = pdpt_entries[pdpt_index] & PHYS_ADDR_MASK;
    uint64_t *pd_entries = (uint64_t *)HHDM_PHYS_TO_VIRT(pd_phys);
    if (!(pd_entries[pd_index] & PAGE_PRESENT))
        return false;

    phys_addr_t pt_phys = pd_entries[pd_index] & PHYS_ADDR_MASK;
    uint64_t *pt_entries = (uint64_t *)HHDM_PHYS_TO_VIRT(pt_phys);
    if (!(pt_entries[pt_index] & PAGE_PRESENT))
        return false;

    return true;
}

phys_addr_t paging_get_physical(phys_addr_t pml4_phys, uint64_t virt_addr)
{
    if (!IS_PAGE_ALIGNED(pml4_phys))
    {
        klog_error("paging_is_mapped: pml4 address is not page alligned 0x%p", pml4_phys);
        return PMM_INVALID_ADDRESS;
    }

    uint64_t pml4_index = PML4_INDEX(virt_addr);
    uint64_t pdpt_index = PDPT_INDEX(virt_addr);
    uint64_t pd_index = PD_INDEX(virt_addr);
    uint64_t pt_index = PT_INDEX(virt_addr);
    uint64_t phys_offset = PAGE_OFFSET(virt_addr);

    uint64_t *pml4_entries = (uint64_t *)HHDM_PHYS_TO_VIRT(pml4_phys);
    if (!(pml4_entries[pml4_index] & PAGE_PRESENT))
        return PMM_INVALID_ADDRESS;

    phys_addr_t pdpt_phys = pml4_entries[pml4_index] & PHYS_ADDR_MASK;
    uint64_t *pdpt_entries = (uint64_t *)HHDM_PHYS_TO_VIRT(pdpt_phys);
    if (!(pdpt_entries[pdpt_index] & PAGE_PRESENT))
        return PMM_INVALID_ADDRESS;

    phys_addr_t pd_phys = pdpt_entries[pdpt_index] & PHYS_ADDR_MASK;
    uint64_t *pd_entries = (uint64_t *)HHDM_PHYS_TO_VIRT(pd_phys);
    if (!(pd_entries[pd_index] & PAGE_PRESENT))
        return PMM_INVALID_ADDRESS;

    phys_addr_t pt_phys = pd_entries[pd_index] & PHYS_ADDR_MASK;
    uint64_t *pt_entries = (uint64_t *)HHDM_PHYS_TO_VIRT(pt_phys);
    if (!(pt_entries[pt_index] & PAGE_PRESENT))
        return PMM_INVALID_ADDRESS;
    return (pt_entries[pt_index] & PHYS_ADDR_MASK) | phys_offset;
}

phys_addr_t paging_get_current_pml4()
{
    return _paging_get_current_dir();
}