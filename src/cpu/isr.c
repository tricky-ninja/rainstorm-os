#include "isr.h"
#include "utils/klog.h"

static isr_handler_fn isr_handlers[256];

static const char *const exceptions[] = {
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
    ""};

void isr_handler(Registers *regs)
{
        uint8_t int_num = (uint8_t)regs->interrupt_num;

        if (isr_handlers[int_num] != NULL)
                isr_handlers[int_num](regs);

        else if (int_num >= 0x20)
        {
                klog_warn("Unhandled Interrupt: %d", int_num);
                return;
        }

        else
        {
                uint64_t cr2;
                asm volatile("mov %%cr2, %0" : "=r"(cr2));

                kprintf("\n--- EXCEPTION: %s (vector=0x%llx) ---\n",
                        exceptions[regs->interrupt_num], regs->interrupt_num);

                kprintf("RIP=0x%016llx  RSP=0x%016llx  RBP=0x%016llx\n",
                        regs->rip, regs->rsp, regs->rbp);
                kprintf("RAX=0x%016llx  RBX=0x%016llx  RCX=0x%016llx  RDX=0x%016llx\n",
                        regs->rax, regs->rbx, regs->rcx, regs->rdx);
                kprintf("RSI=0x%016llx  RDI=0x%016llx\n",
                        regs->rsi, regs->rdi);
                kprintf("R8 =0x%016llx  R9 =0x%016llx  R10=0x%016llx  R11=0x%016llx\n",
                        regs->r8, regs->r9, regs->r10, regs->r11);
                kprintf("R12=0x%016llx  R13=0x%016llx  R14=0x%016llx  R15=0x%016llx\n",
                        regs->r12, regs->r13, regs->r14, regs->r15);
                kprintf("CS =0x%04llx  SS =0x%04llx  RFLAGS=0x%016llx\n",
                        regs->cs, regs->ss, regs->rflags);
                kprintf("CR2=0x%016llx  ERR=0x%016llx\n",
                        cr2, regs->error_code);

                kpanic("unhandled exception");
        }

        klog_debug("interrupt received %llu", regs->interrupt_num);
}


void isr_register_handler(uint8_t isr, isr_handler_fn handler)
{
    isr_handlers[isr] = handler;
}