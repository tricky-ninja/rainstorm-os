/**
 *  ISR Implementation
 *
 *  Handles CPU exceptions (0–31).
 *
 *  Each ISR stub saves registers and calls C handler.
 *
 *  Print debug info on fault.
 *  Author: Sreyas A (TrickyNinja)
 */

#include "isr.h"
#include "printf.h"
#include "io.h"
#include "drivers/x86/pic/pic.h"
#include "klog.h"

#define IRQ_MASTER_OFFSET 0x20
#define IRQ_SLAVE_OFFSET  0x28
#define IRQ_COUNT         8

ISRHandler g_ISRHandlers[256];

static const char* const g_Exceptions[] = {
    "Divide by zero error",
    "Debug",
    "Non-maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack-Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "",
    "x87 Floating-Point Exception",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Control Protection Exception ",
    "",
    "",
    "",
    "",
    "",
    "",
    "Hypervisor Injection Exception",
    "VMM Communication Exception",
    "Security Exception",
    ""
};

void isr_handler(Registers* regs) {
    uint8_t int_num = regs->interrupt_num;

    if (g_ISRHandlers[int_num] != NULL)
        g_ISRHandlers[int_num](regs);
    
    else if (int_num >= 0x20)   // 0x20 --> 32
    {
        klog_warn("Unhandled Interrupt: %d", int_num);
        return;
    }

    else
    {
        klog_critical("Unhandled Excception: %s", g_Exceptions[int_num]);
        io_disableInterrupts();
        asm("hlt"); 
    }

}

void isr_register_handler(uint8_t isr, ISRHandler handler)
{
    g_ISRHandlers[isr] = handler;
}
