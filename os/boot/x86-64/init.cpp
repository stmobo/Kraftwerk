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
		
		// silently drop pages in the 1st MiB
		if((cur_struct->type == 1) &&
		(cur_struct->base_addr >= 0x100000))
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
				
			if((cur_struct->type == 1) &&
			(cur_struct->base_addr >= 0x100000))
				break;
			
			cur += (cur_struct->size+4);
		}
		
		terminal_writestring("\ninit: usable memory at 0x");
		terminal_writehex(cur_struct->base_addr);
		terminal_writestring("\ninit: length = 0x");
		terminal_writehex(cur_struct->length);
		
		mmap[i].beginning = cur_struct->base_addr;
		mmap[i].size = cur_struct->length;
		cur += (cur_struct->size+4);
	}
	
	char* test_mem = (char*)kmalloc(16, 0);
	test_mem[0] = '\n';
	test_mem[1] = 'D';
	test_mem[2] = 'y';
	test_mem[3] = 'n';
	test_mem[4] = 'm';
	test_mem[5] = 'e';
	test_mem[6] = 'm';
	test_mem[7] = 'T';
	test_mem[8] = 's';
	test_mem[9] = 'e';
	test_mem[10] = 't';
	test_mem[11] = 'S';
	test_mem[12] = 't';
	test_mem[13] = 'r';
	test_mem[14] = '!';
	test_mem[15] = '\0';
	
	terminal_writestring(test_mem);
	
	//asm("cli;hlt");
	
	// initialize the rest of the mm's:
	physical_memory::initialize(mmap, n_mem_ranges);
	virtual_memory::initialize();
	
	// Page-fault test.
	uint64_t test_address = 0xFFFFFF8FFFFFFFFF;
	uint32_t* test_pointer = reinterpret_cast<uint32_t*>(test_address);
	
	*test_pointer = 0x007E5700;
	
	terminal_writestring("\ninit: write complete.");
	
	if( *test_pointer != 0x007E5700) {
		terminal_writestring("\ninit: PF test failed.");
	} else {
		terminal_writestring("\ninit: PF test passed.");
	}

	asm("cli;hlt");
	while(true) { asm("pause"); }
}

}