#include "pmm.h"
#include "mm.h"
#include "utils/memory.h"
#include "utils/klog.h"

static uint8_t *bitmap_start;
static size_t bitmap_size;
static size_t total_frames;

uint64_t g_hhdm_offset;

inline static size_t bitmap_size_from_frames(size_t frame_count)
{
    return (frame_count + 7) / 8;
}

void pmm_init(struct limine_memmap_response *memmap, uint64_t hhdm_offset)
{
    g_hhdm_offset = hhdm_offset;

    // calculate bitmap size
    uint64_t highest_addr = 0;
    for (uint64_t i = 0; i < memmap->entry_count; i++)
    {
        struct limine_memmap_entry *entry = memmap->entries[i];
        if (entry->type == LIMINE_MEMMAP_RESERVED || entry->type == LIMINE_MEMMAP_FRAMEBUFFER || entry->type == LIMINE_MEMMAP_BAD_MEMORY) continue;
        uint64_t end = entry->base + entry->length;
        if (end > highest_addr)
            highest_addr = end;
    }
    total_frames = PAGE_COUNT(highest_addr);
    bitmap_size = bitmap_size_from_frames(total_frames);

    // find usable region to place bitmap
    phys_addr_t bitmap_start_phys = 0;
    for (uint64_t i = 0; i < memmap->entry_count; i++)
    {
        struct limine_memmap_entry *entry = memmap->entries[i];
        if (entry->type != LIMINE_MEMMAP_USABLE)
            continue;
        if (entry->length < bitmap_size)
            continue;
        bitmap_start_phys = entry->base;
        break;
    }

    if (bitmap_start_phys == 0)
        kpanic("pmm_init: No usable region found for bitmap");

    bitmap_start = (uint8_t *)HHDM_PHYS_TO_VIRT(bitmap_start_phys);

    memset(bitmap_start, 0xFF, bitmap_size);

    // mark free regions as free
    for (uint64_t i = 0; i < memmap->entry_count; i++)
    {
        struct limine_memmap_entry *entry = memmap->entries[i];
        if (entry->type != LIMINE_MEMMAP_USABLE)
            continue;
        for (uint64_t j = 0; j < entry->length; j += PAGE_SIZE)
            pmm_free_frame(entry->base + j);
    }

    // mark bitmap frames as allocated
    uint64_t bitmap_frame_count = PAGE_COUNT(bitmap_size);
    for (uint64_t frame = 0; frame < bitmap_frame_count; frame++)
    {
        pmm_mark_allocated(bitmap_start_phys + (frame * PAGE_SIZE));
    }

    klog_debug("bitmap_size=%lluKB frames=%llu", bitmap_size, total_frames);
    klog_info("pmm_init: initialized sucessfully");
}

phys_addr_t pmm_alloc_frame()
{
    for (uint64_t byte = 0; byte < bitmap_size; byte++)
    {
        if (bitmap_start[byte] == 0xFF)
            continue;
        for (uint8_t bit = 0; bit < 8; bit++)
        {
            if (((byte * 8) + bit) >= total_frames) break;

            if ((bitmap_start[byte] & (1 << bit)))
                continue;

            bitmap_start[byte] |= (1 << bit);
            return (byte * 8 + bit) * PAGE_SIZE;
        }
    }
    kpanic("pmm_alloc_frame: out of memory");
}

phys_addr_t pmm_alloc_frames(size_t count)
{
    if (count == 0)
        return 0;
    if (count == 1)
        return pmm_alloc_frame();

    size_t run_start = 0;
    size_t run_len = 0;

    for (size_t frame = 0; frame < total_frames; frame++)
    {
        if ((bitmap_start[frame / 8] & (1 << (frame % 8)))) // is the frame allocated
        {
            run_len = 0;
            continue;
        }

        if (run_len == 0)
            run_start = frame;
        run_len++;
        if (run_len != count)
            continue;

        for (size_t i = run_start; i < run_start + count; i++)
            bitmap_start[i / 8] |= (1 << (i % 8));
        return run_start * PAGE_SIZE;
    }

    kpanic("pmm_alloc_frames: out of contiguous memory");
}

void pmm_free_frame(phys_addr_t phys_addr)
{
    if (!IS_PAGE_ALIGNED(phys_addr))
    {
        klog_error("pmm_free_frame: address 0x%llx is not page alligned", phys_addr);
        return;
    }

    uint64_t frame = phys_addr / PAGE_SIZE;
    if (frame >= total_frames)
    {
        klog_error("pmm_free_frame: address 0x%llx is not tracked by the physical memory manager", phys_addr);
        return;
    }

    if (!(bitmap_start[frame / 8] & (1 << (frame % 8))))    // is the frame free
    {
        klog_warn("pmm_free_frame: attempting to free already free frame");
        return;
    }

    bitmap_start[frame / 8] &= ~(1 << (frame % 8));
}

void pmm_free_frames(phys_addr_t phys_addr, size_t count)
{
    for (size_t i = 0; i < count; i++)
    {
        pmm_free_frame(phys_addr + (i * PAGE_SIZE));
    }
}

void pmm_mark_allocated(phys_addr_t phys_addr)
{
    if (!IS_PAGE_ALIGNED(phys_addr))
    {
        klog_error("pmm_mark_allocated: address 0x%llx is not page alligned", phys_addr);
        return;
    }

    uint64_t frame = phys_addr / PAGE_SIZE;
    if (frame >= total_frames)
    {
        klog_error("pmm_mark_allocated: address 0x%llx is not tracked by the physical memory manager", phys_addr);
        return;
    }

    bitmap_start[frame / 8] |= (1 << (frame % 8));
}

bool pmm_is_frame_free(phys_addr_t phys_addr)
{
    if (!IS_PAGE_ALIGNED(phys_addr))
    {
        klog_error("pmm_is_frame_free: address 0x%llx is not page alligned", phys_addr);
        return false;
    }

    uint64_t frame = phys_addr / PAGE_SIZE;
    if (frame >= total_frames)
    {
        klog_warn("pmm_is_frame_free: address 0x%llx is not tracked by the physical memory manager", phys_addr);
        return false;
    }

    return !(bitmap_start[frame / 8] & (1 << (frame % 8)));
}

size_t pmm_get_free_size()
{
    size_t free_frames = 0;
    for (size_t byte = 0; byte < bitmap_size; byte++)
    {
        if (bitmap_start[byte] == 0x00)
        {
            free_frames += 8;
            continue;
        }
        if (bitmap_start[byte] == 0xFF)
            continue;

        for (uint8_t bit = 0; bit < 8; bit++)
        {
            if (!(bitmap_start[byte] & (1 << bit)))
                free_frames++;
        }
    }
    return free_frames * PAGE_SIZE;
}

size_t pmm_get_total_size()
{
    return total_frames * PAGE_SIZE;
}