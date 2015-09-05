#include "interface/types.h"
#include "device/vga.h"

extern "C" {
void kernel_init(void* mb_info, unsigned int magic) {
    // mb_info needs to be adjusted(?)
    
    terminal_initialize();
    
    terminal_writestring("Test!");
    
    asm("cli;hlt");
    while(true) { asm("pause"); }
}
}