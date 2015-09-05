#include "interface/types.h"
#include "arch/x86-64/isr.h"

void halt_err(uint64_t err, address_t eip, uint64_t cs, char* desc) {
    //panic("Kernel mode trap: %s -- error code=%#x", err, desc);
	//kprintf("%s\nerror code=0x%x\nEIP=0x%x\nCS=0x%x", desc, err, eip, cs);
	//logger_flush_buffer();
	while(true) {
        asm volatile("cli\n\t"
                     "hlt\n\t" : : : "memory");
    }
}

// we're calling stuff from asm, use C linkage with everything
extern "C" {
    
void do_isr_div_zero(uint64_t err, address_t eip, uint64_t cs) {
    halt_err(err, eip, cs, "Division by zero error");
}
    
void do_isr_debug(uint64_t err, address_t eip, uint64_t cs) {
    halt_err(err, eip, cs, "Debug interrupt");
}

void do_isr_nmi(uint64_t err, address_t eip, uint64_t cs) {
    halt_err(err, eip, cs, "Non-Maskable interrupt");
}
    
void do_isr_breakpoint(uint64_t err, address_t eip, uint64_t cs) {
    halt_err(err, eip, cs, "Breakpoint");
}
    
void do_isr_overflow(uint64_t err, address_t eip, uint64_t cs) {
    halt_err(err, eip, cs, "Overflow error");
}
    
void do_isr_boundrange(uint64_t err, address_t eip, uint64_t cs) {
    halt_err(err, eip, cs, "BOUND instruction range exceeded");
}
    
void do_isr_invalidop(uint64_t err, address_t eip, uint64_t cs) {
    halt_err(err, eip, cs, "Invalid operation error");
}
    
void do_isr_devnotavail(uint64_t err, address_t eip, uint64_t cs) {
    halt_err(err, eip, cs, "No FPU error");
}
    
void do_isr_dfault(uint64_t err, address_t eip, uint64_t cs) {
    halt_err(err, eip, cs, "Double fault");
}
    
void do_isr_invalidTSS(uint64_t err, address_t eip, uint64_t cs) {
    halt_err(err, eip, cs, "Invalid TSS error");
}
    
void do_isr_segnotpresent(uint64_t err, address_t eip, uint64_t cs) {
    halt_err(err, eip, cs, "Segment PRESENT error");
}
    
void do_isr_stackseg(uint64_t err, address_t eip, uint64_t cs) {
    halt_err(err, eip, cs, "Stack segment fault");
}

void do_isr_gpfault(uint64_t err, address_t eip, uint64_t cs) {
    halt_err(err, eip, cs, "General protection error");
}

void do_isr_pagefault(uint64_t err, address_t eip, uint64_t cs) {
    uint64_t cr2;
    asm volatile("mov %%cr2, %0" : "=g"(cr2) : : "memory");
    //paging_handle_pagefault(err, cr2, eip, cs);
    halt_err(err, eip, cs, "Page fault");
}
    
void do_isr_fpexcept(uint64_t err, address_t eip, uint64_t cs) {
    halt_err(err, eip, cs, "Floating point exception");
}
    
void do_isr_align(uint64_t err, address_t eip, uint64_t cs) {
    halt_err(err, eip, cs, "Alignment check");
}
    
void do_isr_machine(uint64_t err, address_t eip, uint64_t cs) {
    halt_err(err, eip, cs, "Machine check");
}
    
void do_isr_simd_fp(uint64_t err, address_t eip, uint64_t cs) {
    halt_err(err, eip, cs, "SIMD floating point exception");
}
    
void do_isr_virt(uint64_t err, address_t eip, uint64_t cs) {
    halt_err(err, eip, cs, "Virtualization fault");
}
    
void do_isr_security(uint64_t err, address_t eip, uint64_t cs) {
    halt_err(err, eip, cs, "Security fault");
}
    
void do_isr_reserved(uint64_t err, address_t eip, uint64_t cs) {
    halt_err(err, eip, cs, "Reserved exception");
}

void do_isr_test(uint64_t err, address_t eip, uint64_t cs) {
    halt_err(err, eip, cs, "Test interrupt");
}

void do_irq(uint64_t irqn, address_t eip, uint64_t cs) {
    halt_err(irqn, eip, cs, "IRQ");
}

}
