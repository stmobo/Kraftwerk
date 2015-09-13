#pragma once

extern "C" {
    extern void _isr_div_zero(void);
    extern void _isr_debug(void);
    extern void _isr_nmi(void);
    extern void _isr_breakpoint(void);
    extern void _isr_overflow(void);
    extern void _isr_boundrange(void);
    extern void _isr_invalidop(void);
    extern void _isr_devnotavail(void);
    extern void _isr_dfault(void);
    extern void _isr_invalidTSS(void);
    extern void _isr_segnotpresent(void);
    extern void _isr_stackseg(void);
    extern void _isr_gpfault(void);
    extern void _isr_pagefault(void);
    extern void _isr_fpexcept(void);
    extern void _isr_align(void);
    extern void _isr_machine(void);
    extern void _isr_simd_fp(void);
    extern void _isr_virt(void);
    extern void _isr_security(void);
    extern void _isr_reserved(void);
    extern void _isr_test(void);
    extern void _isr_irq_0(void);
    extern void _isr_irq_1(void);
    extern void _isr_irq_2(void);
    extern void _isr_irq_3(void);
    extern void _isr_irq_4(void);
    extern void _isr_irq_5(void);
    extern void _isr_irq_6(void);
    extern void _isr_irq_7(void);
    extern void _isr_irq_8(void);
    extern void _isr_irq_9(void);
    extern void _isr_irq_10(void);
    extern void _isr_irq_11(void);
    extern void _isr_irq_12(void);
    extern void _isr_irq_13(void);
    extern void _isr_irq_14(void);
    extern void _isr_irq_15(void);
    extern void _isr_irq_fe(void);
    extern void _isr_irq_ff(void);
}

extern void add_interrupt_entry( unsigned int, void(*)(void) );
extern void initialize_idt(void);