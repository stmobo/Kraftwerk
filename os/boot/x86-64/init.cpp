#include "interface/types.h"
#include "arch/x86-64/isr.h"
#include "device/vga.h"

extern "C" {

void kernel_init(void* mb_info, unsigned int magic) {
    // mb_info needs to be adjusted(?)
    
    terminal_initialize();
    
    terminal_writestring("Test!");
    
    initialize_idt();
    
    //asm volatile("int $255");
    
    if(magic == 0x2BADB002) {
        terminal_writestring("yes");
    }
    
    asm("cli;hlt");
    while(true) { asm("pause"); }
}

}