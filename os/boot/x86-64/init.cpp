#include "boot/multiboot.h"
#include "interface/types.h"
#include "interface/malloc.h"
#include "interface/physical_mem.h"
#include "interface/virtual_mem.h"
#include "arch/x86-64/isr.h"
#include "device/vga.h"

extern "C" {

void kernel_init(multiboot_info* mb_info, unsigned int magic)
{
	// mb_info needs to be adjusted(?)

	terminal_initialize();

	terminal_writestring("Test!\n");

	initialize_idt();

	//asm volatile("int $255");

	if(magic == 0x2BADB002) {
		terminal_writestring("yes\n");
	}

	malloc_init();
	
	// read number of memory map ranges:
	unsigned int n_mem_ranges = 0;
	uint32_t cur = mb_info->mmap_addr;

	while(cur < (mb_info->mmap_addr + mb_info->mmap_length)) {
		multiboot_mmap_t* cur_struct =
			reinterpret_cast<multiboot_mmap_t*>(cur);
			
		if(cur_struct->type == 1)
			n_mem_ranges++;
		
		cur += (cur_struct->size+4);
	}
	
	physical_memory::boot_mmap_t* mmap = (physical_memory::boot_mmap_t*)
		kmalloc(sizeof(*mmap)*n_mem_ranges, MFLAGS_EMERG);
	
	cur = mb_info->mmap_addr;
	for(unsigned int i=0;i<n_mem_ranges;i++) {
		multiboot_mmap_t* cur_struct = NULL;
		
		while(cur < (mb_info->mmap_addr + mb_info->mmap_length)) {
			cur_struct = reinterpret_cast<multiboot_mmap_t*>(cur);
				
			if(cur_struct->type == 1)
				break;
			
			cur += (cur_struct->size+4);
		}
		
		mmap[i].beginning = cur_struct->base_addr;
		mmap[i].size = cur_struct->length;
		cur += (cur_struct->size+4);
	}
	
	// initialize the rest of the mm's:
	physical_memory::initialize(mmap, n_mem_ranges);
	virtual_memory::initialize();

	asm("cli;hlt");
	while(true) { asm("pause"); }
}

}