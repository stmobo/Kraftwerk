# Multiboot 1
.set ALIGN,     1<<0
.set MEMINFO,   1<<1
.set FLAGS,     ALIGN | MEMINFO
.set MAGIC,     0x1BADB002
.set CHECKSUM,  -(MAGIC+FLAGS)

.code32

.section .multiboot, "aw"
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

.section .boot_tables, "aw"
.align 0x1000
.global PageTable0
.set address, 0x1103 # Global, Read/Write, Present
PageTable0:
    .quad 0
    .rept 511
    .quad address
    .set address, address + 0x1000
    .endr

.align 0x1000
.global KernelPT0
.set address, 0x103 # same flags as above
KernelPT0:
    .rept 512
    .quad address
    .set address, address + 0x1000
    .endr
    
.global BootPT1
.set address, 0x200103 # same flags as above
BootPT1:
    .rept 512
    .quad address
    .set address, address + 0x1000
    .endr

.align 0x1000
.global BootPD
BootPD:
    .quad (PageTable0+1)
    .quad (BootPT1+1)
    .rept 510
    .quad 0
    .endr
    
.align 0x1000
.global KernelPD
KernelPD:
    .quad (KernelPT0+1)
    .rept 511
    .quad 0
    .endr
    
.align 0x1000
.global BootPDPT
BootPDPT:
    .quad (BootPD+1)
    .rept 511
    .quad 0
    .endr

.align 0x1000
.global KernelPDPT
KernelPDPT:
    .quad (KernelPD+1)
    .rept 511
    .quad 0
    .endr
    
.align 0x1000
.global BootPML4T
BootPML4T:
    .quad (BootPDPT+1)
    .rept 509
    .quad 0
    .endr
    .quad (BootPML4T+1)
    .quad (KernelPDPT+1)

.section .bootstrap_stack, "aw", @nobits
.align 32
stack_bottom:
.skip 16384
stack_top:

.section .boot_tables, "aw"
.align 32
BootGDT:
    # null descriptor
    .word 0
    .word 0
    .byte 0
    .byte 0
    .byte 0
    .byte 0
    # kcode descriptor
    .word 0xFFFF      # limit low
    .word 0           # base low
    .byte 0           # base continued
    .byte 0b10011010  # access (readable, executable, present)
    .byte 0b00101111  # limit high / flags
    .byte 0           # base high
    # kdata descriptor
    .word 0xFFFF           
    .word 0          
    .byte 0           
    .byte 0b10010010  # (readable, nonexecutable, present)
    .byte 0b00001111
    .byte 0
    # ucode descriptor
    .word 0xFFFF          
    .word 0          
    .byte 0          
    .byte 0b11111010  # (priv. level 3)
    .byte 0b00101111  
    .byte 0           
    # udata descriptor
    .word 0xFFFF           
    .word 0          
    .byte 0           
    .byte 0b11110010  
    .byte 0b00001111
    .byte 0
    
.align 32
BootGDTPtr:
    .space 2
BootGDTLim:
    .space 2
BootGDTBse:
    .space 4
    
.align 32
BootIDT:
    .rept 16
    .word 0 # fill at runtime (displacement 0)
    .word 0x08
    .byte 0
    .byte 0x1F
    .word 0 # fill at runtime (displacement 4)
    .endr
.align 32
BootIDTPtr:
    .word (8*16)-1
    .long BootIDT
    
.section .entry, "ax"
.global exception
.global start
.global set_isr
exception:
    cli
    hlt
    jmp exception

# 1st argument: index 
# 2nd argument: address
set_isr:
    # get pointer into IDT
    # entry = BootIDT + (8*index)
    mov $8, %eax
    mov 4(%esp), %ebx # get index
    mul %ebx # eax = eax * ebx = 8 * index
    add $BootIDT, %eax # eax = BootIDT + eax = BootIDT + (8 * index)
    
    mov (%esp), %ebx # get address
    mov (%esp), %ecx
    
    shr $16, %ecx # get high bits
    
    mov (%eax), %bx
    mov 4(%eax), %cx
    
    ret
    
start:
    cli
    
    movl $stack_top, %esp
    movl $stack_top, %ebp
    
    push %eax # push magic value
    push %ebx # push multiboot struct address (physical)
    
    lidt BootIDTPtr
    
    mov $BootPML4T, %eax
    mov %eax, %cr3
    
    # Enable paging features:
    mov %cr4, %eax
    or $32, %eax # PAE Paging
    or $8, %eax  # Global Page Sharing
    mov %eax, %cr4
    
    # Enable Long Mode
    mov $0xC0000080, %ecx
    rdmsr
    or $0x100, %eax
    wrmsr
    
    # Enable paging
    mov %cr0, %ecx
    or $0x80000000, %ecx
    mov %ecx, %cr0
    
    # load GDT
    mov $BootGDT, %eax
    mov $39, %edx
    
    mov %dx, BootGDTLim
    mov %eax, BootGDTBse
    
    lgdt BootGDTLim
    
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss
    
    .intel_syntax
    jmp 0x08:_reload_cs # set CS
    .att_syntax
.code64
_reload_cs:
    # jump to higher-half code
    movabs $_start_kernel_2, %rax
    jmp *%rax
 
.section .text, "ax"
.global kernel_init
_start_kernel_2:
    call kernel_init
    
    pop %rbx
    pop %rax
    