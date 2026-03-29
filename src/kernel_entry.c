#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "limine.h"
#include "utils/memory.h"
#include "drivers/serial/serial.h"
#include "utils/printf.h"
#include "utils/klog.h"
#include "drivers/screen/framebuffer.h"
#include "cpu/gdt.h"
#include "cpu/idt.h"
#include "cpu/isr.h"
#include "memory/pmm.h"
#include "memory/paging.h"

#define SECTION(x) __attribute__((used, section(x)))

SECTION(".limine_requests")
static volatile uint64_t limine_base_revision[] = LIMINE_BASE_REVISION(5);

SECTION(".limine_requests")
static volatile struct limine_framebuffer_request fb_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0,
    .response = NULL};

SECTION(".limine_requests")
static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST_ID,
    .revision = 0,
    .response = NULL};

SECTION(".limine_requests")
static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST_ID,
    .revision = 0,
    .response = NULL};

SECTION(".limine_requests")
static volatile struct limine_executable_address_request address_request = {
    .id = LIMINE_EXECUTABLE_ADDRESS_REQUEST_ID,
    .revision = 0,
    .response = NULL};

SECTION(".limine_requests_start")
static volatile uint64_t limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;

SECTION(".limine_requests_end")
static volatile uint64_t limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;

extern uint8_t _bss_start;
extern uint8_t _bss_end;

extern uint8_t _LIMINE_BEGIN;
extern uint8_t _LIMINE_END;

extern uint8_t _TEXT_BEGIN;
extern uint8_t _TEXT_END;

extern uint8_t _RODATA_BEGIN;
extern uint8_t _RODATA_END;

extern uint8_t _DATA_BEGIN;
extern uint8_t _DATA_END;

void validate_limine_or_halt();
phys_addr_t memory_init();
void screen_init(phys_addr_t kernel_pml4);


static void halt()
{
    for (;;)
    {
        asm volatile("cli");
        asm volatile("hlt");
    }
}


void kmain()
{

    size_t bss_size = (size_t)((char *)&_bss_end - (char *)&_bss_start);
    memset(&_bss_start, 0, bss_size);

    validate_limine_or_halt();

    int serial_sink_id = serial_init(SERIAL_COM1_BASE);
    kprintf_set_default_sink(serial_sink_id);
    klog_set_level(klog_level_debug);

    kprintf("Hello from RainstormOS :)\n");

    gdt_init();
    idt_init();
    klog_info("gdt and idt are working");

    phys_addr_t kernel_pml4 = memory_init();
    screen_init(kernel_pml4);

    klog_info("Halting....");
    halt();
}

void validate_limine_or_halt()
{
    if (LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision) == false)
        halt();
    if (fb_request.response == NULL || fb_request.response->framebuffer_count < 1)
        halt();
    if (memmap_request.response == NULL)
        halt();
    if (hhdm_request.response == NULL)
        halt();
    if (address_request.response == NULL)
        halt();
}

phys_addr_t memory_init()
{
    pmm_init(memmap_request.response, hhdm_request.response->offset);
    phys_addr_t kernel_physical_base = address_request.response->physical_base;
    phys_addr_t kernel_virtual_base = address_request.response->virtual_base;
    klog_debug("kernel_virtual_base=0x%p, kernel_physical_base=0x%p", kernel_virtual_base, kernel_physical_base);
    phys_addr_t kernel_pml4 = paging_init(kernel_physical_base, kernel_virtual_base, (kernel_segment_info_t){
        .limine_start = &_LIMINE_BEGIN,
        .limine_end = (uint8_t*)PAGE_ALIGN_UP((uintptr_t)&_LIMINE_END),
        .text_start = &_TEXT_BEGIN,
        .text_end = (uint8_t*)PAGE_ALIGN_UP((uintptr_t)&_TEXT_END),
        .rodata_start = &_RODATA_BEGIN,
        .rodata_end = (uint8_t*)PAGE_ALIGN_UP((uintptr_t)&_RODATA_END),
        .data_start = &_DATA_BEGIN,
        .data_end =  (uint8_t*)PAGE_ALIGN_UP((uintptr_t)&_DATA_END)
    });

    klog_info("Memory management stuff is working");

    return kernel_pml4;
}

void screen_init(phys_addr_t kernel_pml4)
{
    struct limine_framebuffer *fb = fb_request.response->framebuffers[0];
    if (fb->memory_model != 1)
        kpanic("screen_init: framebuffer memory_model is %u, expected 1", fb->memory_model);

    size_t fb_size = fb->pitch * fb->height;
    uint64_t fb_flags = PAGE_CACHE_DISABLE | PAGE_WRITABLE | PAGE_NO_EXECUTE | PAGE_GLOBAL | PAGE_PRESENT;
    phys_addr_t fb_phys = HHDM_VIRT_TO_PHYS((uint64_t)fb->address);
    if (pmm_is_frame_free(fb_phys))
        kpanic("screen_init: framebuffer memory is free");
    

    paging_map_range(kernel_pml4, (uint64_t)fb->address, HHDM_VIRT_TO_PHYS((uint64_t)fb->address), fb_size, fb_flags);

    fb_init((framebuffer_t){
        .address = fb->address,
        .height = fb->height,
        .width = fb->width,
        .pitch = fb->pitch});

    fb_clear_screen((fb_color_t){40, 40, 60});
    klog_info("Framebuffer up and working");
}