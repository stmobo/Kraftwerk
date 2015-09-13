#include "interface/types.h"
#include "arch/x86-64/isr.h"
#include "device/vga.h"

void halt_err(uint64_t err, vmem_t rip, uint16_t cs, const char* desc) {
    //panic("Kernel mode trap: %s -- error code=%#x", err, desc);
	//kprintf("%s\nerror code=0x%x\nrip=0x%x\nCS=0x%x", desc, err, rip, cs);
	//logger_flush_buffer();
    
    terminal_writestring(desc);
    
	while(true) {
        asm volatile("cli\n\t"
                     "hlt\n\t" : : : "memory");
    }
}

// we're calling stuff from asm, use C linkage with everything
extern "C" {
    
void do_isr_div_zero(uint64_t err, vmem_t rip, uint16_t cs) {
    halt_err(err, rip, cs, "Division by zero error");
}
    
void do_isr_debug(uint64_t err, vmem_t rip, uint16_t cs) {
    halt_err(err, rip, cs, "Debug interrupt");
}

void do_isr_nmi(uint64_t err, vmem_t rip, uint16_t cs) {
    halt_err(err, rip, cs, "Non-Maskable interrupt");
}
    
void do_isr_breakpoint(uint64_t err, vmem_t rip, uint16_t cs) {
    halt_err(err, rip, cs, "Breakpoint");
}
    
void do_isr_overflow(uint64_t err, vmem_t rip, uint16_t cs) {
    halt_err(err, rip, cs, "Overflow error");
}
    
void do_isr_boundrange(uint64_t err, vmem_t rip, uint16_t cs) {
    halt_err(err, rip, cs, "BOUND instruction range exceeded");
}
    
void do_isr_invalidop(uint64_t err, vmem_t rip, uint16_t cs) {
    halt_err(err, rip, cs, "Invalid operation error");
}
    
void do_isr_devnotavail(uint64_t err, vmem_t rip, uint16_t cs) {
    halt_err(err, rip, cs, "No FPU error");
}
    
void do_isr_dfault(uint64_t err, vmem_t rip, uint16_t cs) {
    halt_err(err, rip, cs, "Double fault");
}
    
void do_isr_invalidTSS(uint64_t err, vmem_t rip, uint16_t cs) {
    halt_err(err, rip, cs, "Invalid TSS error");
}
    
void do_isr_segnotpresent(uint64_t err, vmem_t rip, uint16_t cs) {
    halt_err(err, rip, cs, "Segment PRESENT error");
}
    
void do_isr_stackseg(uint64_t err, vmem_t rip, uint16_t cs) {
    halt_err(err, rip, cs, "Stack segment fault");
}

void do_isr_gpfault(uint64_t err, vmem_t rip, uint16_t cs) {
    halt_err(err, rip, cs, "General protection error");
}

void do_isr_pagefault(uint64_t err, vmem_t rip, uint16_t cs) {
    uint64_t cr2;
    asm volatile("mov %%cr2, %0" : "=g"(cr2) : : "memory");
    //paging_handle_pagefault(err, cr2, rip, cs);
    halt_err(err, rip, cs, "Page fault");
}
    
void do_isr_fpexcept(uint64_t err, vmem_t rip, uint16_t cs) {
    halt_err(err, rip, cs, "Floating point exception");
}
    
void do_isr_align(uint64_t err, vmem_t rip, uint16_t cs) {
    halt_err(err, rip, cs, "Alignment check");
}
    
void do_isr_machine(uint64_t err, vmem_t rip, uint16_t cs) {
    halt_err(err, rip, cs, "Machine check");
}
    
void do_isr_simd_fp(uint64_t err, vmem_t rip, uint16_t cs) {
    halt_err(err, rip, cs, "SIMD floating point exception");
}
    
void do_isr_virt(uint64_t err, vmem_t rip, uint16_t cs) {
    halt_err(err, rip, cs, "Virtualization fault");
}
    
void do_isr_security(uint64_t err, vmem_t rip, uint16_t cs) {
    halt_err(err, rip, cs, "Security fault");
}
    
void do_isr_reserved(uint64_t err, vmem_t rip, uint16_t cs) {
    halt_err(err, rip, cs, "Reserved exception");
}

void do_isr_test(uint64_t err, vmem_t rip, uint16_t cs) {
    halt_err(err, rip, cs, "Test interrupt");
}

void do_irq(uint64_t irqn, vmem_t rip, uint16_t cs) {
    halt_err(irqn, rip, cs, "IRQ");
}

}

struct idt_entry {
    uint16_t offset_low;    // offset bits 0 - 15
    uint16_t code_selector;
    uint8_t  zero;
    uint8_t  type;          // Present / Priv Level / 0 / Gate Type
    uint16_t offset_middle; // offset bits 16 - 31
    uint32_t offset_high;   // offset bits 32 - 63
    uint32_t zero2;
} __attribute__((packed));

idt_entry idt[256];

void add_interrupt_entry( unsigned int int_no, void(*handler_func)(void) ) {
    idt_entry new_entry;
    
    if( int_no > 255 ) {
        return;
    }
    
    new_entry.offset_low = (uint16_t)( reinterpret_cast<uint64_t>(handler_func) & 0xFFFF );
    new_entry.code_selector = 0x08;
    new_entry.zero = 0;
    new_entry.type = 0x8F;
    new_entry.offset_middle = (uint16_t)( (reinterpret_cast<uint64_t>(handler_func) >> 16) & 0xFFFF );
    new_entry.offset_high = (uint32_t)( (reinterpret_cast<uint64_t>(handler_func) >> 32) & 0xFFFFFFFF );
    new_entry.zero2 = 0;
    
    idt[int_no] = new_entry;
}

void initialize_idt() {
    // Exceptions:
    add_interrupt_entry(0, _isr_div_zero);
    add_interrupt_entry(1, _isr_debug);
    add_interrupt_entry(2, _isr_nmi);
    add_interrupt_entry(3, _isr_breakpoint);
    add_interrupt_entry(4, _isr_overflow);
    add_interrupt_entry(5, _isr_boundrange);
    add_interrupt_entry(6, _isr_invalidop);
    add_interrupt_entry(7, _isr_devnotavail);
    add_interrupt_entry(8, _isr_dfault);
    add_interrupt_entry(10, _isr_invalidTSS);
    add_interrupt_entry(11, _isr_segnotpresent);
    add_interrupt_entry(12, _isr_stackseg);
    add_interrupt_entry(13, _isr_gpfault);
    add_interrupt_entry(14, _isr_pagefault);
    add_interrupt_entry(15, _isr_reserved);
    add_interrupt_entry(16, _isr_fpexcept);
    add_interrupt_entry(17, _isr_align);
    add_interrupt_entry(18, _isr_machine);
    add_interrupt_entry(19, _isr_simd_fp);
    add_interrupt_entry(20, _isr_virt);
    for(unsigned int i=21;i<32;i++) {
        add_interrupt_entry( i, _isr_reserved );
    }
    
    add_interrupt_entry(30, _isr_security);
    
    add_interrupt_entry( (32+0), _isr_irq_0 );
    add_interrupt_entry( (32+1), _isr_irq_1 );
    add_interrupt_entry( (32+2), _isr_irq_2 );
    add_interrupt_entry( (32+3), _isr_irq_3 );
    add_interrupt_entry( (32+4), _isr_irq_4 );
    add_interrupt_entry( (32+5), _isr_irq_5 );
    add_interrupt_entry( (32+6), _isr_irq_6 );
    add_interrupt_entry( (32+7), _isr_irq_7 );
    
    add_interrupt_entry( (32+8), _isr_irq_8 );
    add_interrupt_entry( (32+9), _isr_irq_9 );
    add_interrupt_entry( (32+10), _isr_irq_10 );
    add_interrupt_entry( (32+11), _isr_irq_11 );
    add_interrupt_entry( (32+12), _isr_irq_12 );
    add_interrupt_entry( (32+13), _isr_irq_13 );
    add_interrupt_entry( (32+14), _isr_irq_14 );
    add_interrupt_entry( (32+15), _isr_irq_15 );
    
    add_interrupt_entry(255, _isr_test);
    
    // now LIDT:
    struct {
        uint16_t limit;
        uint64_t pointer;
    } __attribute__((packed)) idt_ptr;
    
    idt_ptr.limit = 0x1000;
    idt_ptr.pointer = reinterpret_cast<uint64_t>( idt );
    
    asm volatile("lidt %0" : : "m"(idt_ptr));
}