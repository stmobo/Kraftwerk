# generic wrapper stuff
_isr_call_cpp_func:
    # make sure we don't mess up any running programs
    #-40:  Return RFLAGS
    #-32:  Return CS
    #-24:  Return RIP
    #-16:  Saved RAX
    #-8 :  Exception number / error code
    # RSP: Function address
    
    push %r8
    push %r9
    push %r10
    push %r11
    push %r12
    push %r13
    push %r14
    push %r15
    push %rcx
    push %rdx
    push %rbx
    push %rsp
    push %rbp
    push %rsi
    push %rdi
    
    # -160: RFLAGS
    # -152: Return CS
    # -144: Return Address
    # -136: ErrCode
    # -128: RAX
    # -120: Exception Handler Address
    # -112: R8
    # -104: R9
    # -96 : R10
    # -88 : R11
    # -80 : R12
    # -72 : R13
    # -64 : R14
    # -56 : R15
    # -48 : RCX
    # -40 : RDX
    # -32 : RBX
    # -24 : RSP (pre-call)
    # -16 : RBP
    # -8  : RSI
    # RSP : RDI
    
    mov 120(%rsp), %rax # get func addr
    mov 136(%rsp), %rdi # get err code (ISR vector no. if no err code)
    mov 144(%rsp), %rsi # saved RIP
    mov 152(%rsp), %rdx # saved CS
    call *%rax # call cpp handler func
    
    pop %rdi
    pop %rsi
    pop %rbp
    pop %rsp
    pop %rbx
    pop %rdx
    pop %rcx
    pop %r15
    pop %r14
    pop %r13
    pop %r12
    pop %r11
    pop %r10
    pop %r9
    pop %r8
    
    retq
    
# Before any ISR code runs, our stack looks like:
# ... program stack ...
# Saved EFLAGS
# Saved CS
# Saved RIP
# Error code (if given)
#
# %rsp will be pointing to either the saved RIP or to the error code, if given.
    
_isr_div_zero:
    pushq %rax # preserve value
    pushq $0
    
    movabs $do_isr_div_zero, %rax
    pushq %rax # push address of handler
    
    jmp _isr_call_cpp_func
    
    add $16, %rsp # discard err no. and handler addr.
    popq %rax     # restore rax
    iretq
    
_isr_debug:
    pushq %rax
    pushq $1
    
    movabs $do_isr_debug, %rax
    pushq %rax # push address of handler
    
    jmp _isr_call_cpp_func
    
    add $16, %rsp
    popq %rax
    iretq

_isr_nmi: # code 2, do_isr_nmi
    pushq %rax
    pushq $2
    
    movabs $do_isr_nmi, %rax
    pushq %rax # push address of handler
    
    jmp _isr_call_cpp_func
    
    add $16, %rsp
    popq %rax
    iretq
    
_isr_breakpoint: # code 3, do_isr_breakpoint
    pushq %rax
    pushq $3
    
    movabs $do_isr_breakpoint, %rax
    pushq %rax # push address of handler
    
    jmp _isr_call_cpp_func
    
    add $16, %rsp
    popq %rax
    iretq
    
_isr_overflow: # code 4, do_isr_overflow
    pushq %rax
    pushq $4
    
    movabs $do_isr_overflow, %rax
    pushq %rax # push address of handler
    
    jmp _isr_call_cpp_func
    
    add $16, %rsp
    popq %rax
    iretq
    
_isr_boundrange: # code 5, do_isr_boundrange
    pushq %rax
    pushq $5
    
    movabs $do_isr_boundrange, %rax
    pushq %rax # push address of handler
    
    jmp _isr_call_cpp_func
    
    add $16, %rsp
    popq %rax
    iretq
    
_isr_invalidop: # code 6, do_isr_invalidop
    pushq %rax
    pushq $6
    
    movabs $do_isr_invalidop, %rax
    pushq %rax # push address of handler
    
    jmp _isr_call_cpp_func
    
    add $16, %rsp
    popq %rax
    iretq
    
_isr_devnotavail: # code 7, do_isr_devnotavail
    pushq %rax
    pushq $7
    
    movabs $do_isr_devnotavail, %rax
    pushq %rax # push address of handler
    
    jmp _isr_call_cpp_func
    
    add $16, %rsp
    popq %rax
    iretq
    
_isr_dfault: # code ??, do_isr_dfault
    pushq %rax
    
    movabs $do_isr_dfault, %rax
    pushq %rax # push address of handler
    
    jmp _isr_call_cpp_func
    
    add $16, %rsp
    popq %rax
    iretq
    
_isr_invalidTSS: # code ??, do_isr_invalidTSS
    pushq %rax
    
    movabs $do_isr_invalidTSS, %rax
    pushq %rax # push address of handler
    
    jmp _isr_call_cpp_func
    
    add $16, %rsp
    popq %rax
    iretq
    
_isr_segnotpresent: # code ??, do_isr_segnotpresent
    pushq %rax
    
    movabs $do_isr_segnotpresent, %rax
    pushq %rax # push address of handler
    
    jmp _isr_call_cpp_func
    
    add $16, %rsp
    popq %rax
    iretq
    
_isr_stackseg: # code ??, do_isr_stackseg
    pushq %rax
    
    movabs $do_isr_stackseg, %rax
    pushq %rax # push address of handler
    
    jmp _isr_call_cpp_func
    
    add $16, %rsp
    popq %rax
    iretq
    
_isr_gpfault: # code ??, do_isr_gpfault
    pushq %rax
    
    movabs $do_isr_gpfault, %rax
    pushq %rax # push address of handler
    
    jmp _isr_call_cpp_func
    
    add $16, %rsp
    popq %rax
    iretq
    
_isr_pagefault: # code ??, do_isr_pagefault
    pushq %rax
    
    movabs $do_isr_pagefault, %rax
    pushq %rax # push address of handler
    
    jmp _isr_call_cpp_func
    
    add $16, %rsp
    popq %rax
    iretq
    
_isr_fpexcept: # code 15, do_isr_fpexcept
    pushq %rax
    pushq $15
    
    movabs $do_isr_fpexcept, %rax
    pushq %rax # push address of handler
    
    jmp _isr_call_cpp_func
    
    add $16, %rsp
    popq %rax
    iretq
    
_isr_align: # code ??, do_isr_align
    pushq %rax
    
    movabs $do_isr_align, %rax
    pushq %rax # push address of handler
    
    jmp _isr_call_cpp_func
    
    add $16, %rsp
    popq %rax
    iretq
    
_isr_machine: # code 17, do_isr_machine
    pushq %rax
    pushq $17
    
    movabs $do_isr_machine, %rax
    pushq %rax # push address of handler
    
    jmp _isr_call_cpp_func
    
    add $16, %rsp
    popq %rax
    iretq
    
_isr_simd_fp: # code 18, do_isr_simd_fp
    pushq %rax
    pushq $18
    
    movabs $do_isr_simd_fp, %rax
    pushq %rax # push address of handler
    
    jmp _isr_call_cpp_func
    
    add $16, %rsp
    popq %rax
    iretq
    
_isr_virt: # code 19, do_isr_virt
    pushq %rax
    pushq $19
    
    movabs $do_isr_virt, %rax
    pushq %rax # push address of handler
    
    jmp _isr_call_cpp_func
    
    add $16, %rsp
    popq %rax
    iretq
    
_isr_security: # code 20, do_isr_security
    pushq %rax
    pushq $20
    
    movabs $do_isr_security, %rax
    pushq %rax # push address of handler
    
    jmp _isr_call_cpp_func
    
    add $16, %rsp
    popq %rax
    iretq
    
_isr_reserved: # code 0xFF, do_isr_reserved
    pushq %rax
    pushq $0xFF
    
    movabs $do_isr_reserved, %rax
    pushq %rax # push address of handler
    
    jmp _isr_call_cpp_func
    
    add $16, %rsp
    popq %rax
    iretq
    
_isr_test: # code 0x8D, do_isr_test
    pushq %rax
    pushq $0x8D
    
    movabs $do_isr_test, %rax
    pushq %rax # push address of handler
    
    jmp _isr_call_cpp_func
    
    add $16, %rsp
    popq %rax
    iretq

# special treatment for this IRQ
#_isr_irq_0:
#   # decrement the task switch timer
#    pushq %eax
#    mov (multitasking_enabled), %eax
#    cmp $0, %eax
#    je .__isr_irq_0_no_multitasking
#    
#    mov (multitasking_timeslice_tick_count), %eax
#    # is it time to switch?
#    cmp $0, %eax
#    jne .__isr_irq_0_no_ctext_switch # do a "normal" irq call if it isn't
#    
#    pop %eax
#    movl $0, (as_syscall)
#    movl $0, (syscall_num)
#    movl $0, (syscall_arg1)
#    movl $0, (syscall_arg2)
#    movl $0, (syscall_arg3)
#    movl $0, (syscall_arg4)
#    movl $0, (syscall_arg5)
#    jmp __multitasking_kmode_entry # do note that __multitasking_kmode_entry calls the irq handler in our stead.
#    # not falling through -- __multitasking_kmode_entry does the iret itself
#    
#__isr_irq_0_no_ctext_switch:
#    dec %eax
#    mov %eax, (multitasking_timeslice_tick_count) # store new tick count
#__isr_irq_0_no_multitasking:
#    pop %eax
#    pushq $0
#    pushq $do_irq
#    jmp _isr_call_cpp_func
    
_isr_irq_0:
    pushq %rax
    pushq $0
    
    movabs $do_irq, %rax
    pushq %rax
    
    jmp _isr_call_cpp_func
    
    add $16, %rsp
    popq %rax
    iretq
    
_isr_irq_1:
    pushq %rax
    pushq $1
    
    movabs $do_irq, %rax
    pushq %rax
    
    jmp _isr_call_cpp_func
    
    add $16, %rsp
    popq %rax
    iretq
    
_isr_irq_2:
    pushq %rax
    pushq $2
    
    movabs $do_irq, %rax
    pushq %rax
    
    jmp _isr_call_cpp_func
    
    add $16, %rsp
    popq %rax
    iretq
    
_isr_irq_3:
    pushq %rax
    pushq $3
    
    movabs $do_irq, %rax
    pushq %rax
    
    jmp _isr_call_cpp_func
    
    add $16, %rsp
    popq %rax
    iretq
    
_isr_irq_4:
    pushq %rax
    pushq $4
    
    movabs $do_irq, %rax
    pushq %rax
    
    jmp _isr_call_cpp_func
    
    add $16, %rsp
    popq %rax
    iretq

_isr_irq_5:
    pushq %rax
    pushq $5
    
    movabs $do_irq, %rax
    pushq %rax
    
    jmp _isr_call_cpp_func
    
    add $16, %rsp
    popq %rax
    iretq
    
_isr_irq_6:
    pushq %rax
    pushq $6
    
    movabs $do_irq, %rax
    pushq %rax
    
    jmp _isr_call_cpp_func
    
    add $16, %rsp
    popq %rax
    iretq
    
_isr_irq_7:
    pushq %rax
    pushq $7
    
    movabs $do_irq, %rax
    pushq %rax
    
    jmp _isr_call_cpp_func
    
    add $16, %rsp
    popq %rax
    iretq
    
_isr_irq_8:
    pushq %rax
    pushq $8
    
    movabs $do_irq, %rax
    pushq %rax
    
    jmp _isr_call_cpp_func
    
    add $16, %rsp
    popq %rax
    iretq
    
_isr_irq_9:
    pushq %rax
    pushq $9
    
    movabs $do_irq, %rax
    pushq %rax
    
    jmp _isr_call_cpp_func
    
    add $16, %rsp
    popq %rax
    iretq
    
_isr_irq_10:
    pushq %rax
    pushq $10
    
    movabs $do_irq, %rax
    pushq %rax
    
    jmp _isr_call_cpp_func
    
    add $16, %rsp
    popq %rax
    iretq
    
_isr_irq_11:
    pushq %rax
    pushq $11
    
    movabs $do_irq, %rax
    pushq %rax
    
    jmp _isr_call_cpp_func
    
    add $16, %rsp
    popq %rax
    iretq
    
_isr_irq_12:
    pushq %rax
    pushq $12
    
    movabs $do_irq, %rax
    pushq %rax
    
    jmp _isr_call_cpp_func
    
    add $16, %rsp
    popq %rax
    iretq
    
_isr_irq_13:
    pushq %rax
    pushq $13
    
    movabs $do_irq, %rax
    pushq %rax
    
    jmp _isr_call_cpp_func
    
    add $16, %rsp
    popq %rax
    iretq
    
_isr_irq_14:
    pushq %rax
    pushq $14
    
    movabs $do_irq, %rax
    pushq %rax
    
    jmp _isr_call_cpp_func
    
    add $16, %rsp
    popq %rax
    iretq
    
_isr_irq_15:
    pushq %rax
    pushq $15
    
    movabs $do_irq, %rax
    pushq %rax
    
    jmp _isr_call_cpp_func
    
    add $16, %rsp
    popq %rax
    iretq

 _isr_irq_fe:
    pushq %rax
    pushq $254
    
    movabs $do_irq, %rax
    pushq %rax
    
    jmp _isr_call_cpp_func
    
    add $16, %rsp
    popq %rax
    iretq
    
 _isr_irq_ff:
    pushq %rax
    pushq $255
    
    movabs $do_irq, %rax
    pushq %rax
    
    jmp _isr_call_cpp_func
    
    add $16, %rsp
    popq %rax
    iretq

_isr_irq_generic:
    pushq %rax
    pushq $16
    
    movabs $do_irq, %rax
    pushq %rax
    
    jmp _isr_call_cpp_func
    
    add $16, %rsp
    popq %rax
    iretq

.globl _isr_div_zero
.globl _isr_debug
.globl _isr_nmi
.globl _isr_breakpoint
.globl _isr_overflow
.globl _isr_boundrange
.globl _isr_invalidop
.globl _isr_devnotavail
.globl _isr_dfault
.globl _isr_invalidTSS
.globl _isr_segnotpresent
.globl _isr_stackseg
.globl _isr_gpfault
.globl _isr_pagefault
.globl _isr_fpexcept
.globl _isr_align
.globl _isr_machine
.globl _isr_simd_fp
.globl _isr_virt
.globl _isr_security
.globl _isr_reserved
.globl _isr_test
.globl _isr_irq_0
.globl _isr_irq_1
.globl _isr_irq_2
.globl _isr_irq_3
.globl _isr_irq_4
.globl _isr_irq_5
.globl _isr_irq_6
.globl _isr_irq_7
.globl _isr_irq_8
.globl _isr_irq_9
.globl _isr_irq_10
.globl _isr_irq_11
.globl _isr_irq_12
.globl _isr_irq_13
.globl _isr_irq_14
.globl _isr_irq_15
.globl _isr_irq_fe
.globl _isr_irq_ff
.globl _isr_irq_generic