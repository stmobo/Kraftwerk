#include "arch/x86-64/page_manip.h"
#include "device/vga.h"
#include "interface/types.h"
#include "interface/physical_mem.h"
#include "interface/paging.h"

// Ensures that a page table exists before writing to it:
void mem_create_pagetable( vmem_t vaddr ) {
	// check if the PML4T entry is there
	if( !(MEM_PML4T_ADDR[ MEM_PML4_INDEX(vaddr) ] & 1) ) {
		// not present, make a new PDPT
		pmem_t pdpt = physical_memory::allocate(1);
		MEM_PML4T_ADDR[ MEM_PML4_INDEX(vaddr) ] = (pdpt | 0x101);
		terminal_writestring("\npaging: pdpt at 0x");
		terminal_writehex(pdpt);
		terminal_writestring(" mapped");
	}
	// now it is; check if the PDPT entry is there
	if( !(MEM_PDPT_ADDR(vaddr)[ MEM_PDPT_INDEX(vaddr) ] & 1) ) {
		pmem_t pd = physical_memory::allocate(1);
		MEM_PDPT_ADDR(vaddr)[ MEM_PDPT_INDEX(vaddr) ] = (pd | 0x101);
		terminal_writestring("\npaging: pd at 0x");
		terminal_writehex(pd);
		terminal_writestring(" mapped");
	}
	// check for PD entry:
	if( !(MEM_PDIR_ADDR(vaddr)[ MEM_PDIR_INDEX(vaddr) ] & 1) ) {
		pmem_t pt = physical_memory::allocate(1);
		MEM_PDIR_ADDR(vaddr)[ MEM_PDIR_INDEX(vaddr) ] = (pt | 0x101);
		terminal_writestring("\npaging: pt at 0x");
		terminal_writehex(pt);
		terminal_writestring(" mapped");
	}
}

// As above, but only checks for existence; doesn't create tables
bool mem_check_pagetable( vmem_t vaddr ) {
	if( !(MEM_PML4T_ADDR[ MEM_PML4_INDEX(vaddr) ] & 1) ) {
		return false;
	}
	if( !(MEM_PDPT_ADDR(vaddr)[ MEM_PDPT_INDEX(vaddr) ] & 1) ) {
		return false;
	}
	if( !(MEM_PDIR_ADDR(vaddr)[ MEM_PDIR_INDEX(vaddr) ] & 1) ) {
		return false;
	}
	return true;
}

page_entry mem_get_mmu_entry( vmem_t vaddr ) {
	if( !mem_check_pagetable(vaddr) ) {
		return page_entry(0);
	}
	return page_entry(MEM_PTBL_ADDR(vaddr)[ MEM_PTBL_INDEX(vaddr) ]);
}

pmem_t mem_get_physical_address( vmem_t vaddr ) {
	if( !mem_check_pagetable(vaddr) ) {
		return 0;
	}
	return MEM_PTBL_ADDR(vaddr)[ MEM_PTBL_INDEX(vaddr) ] & ~(0xFFF);
}

pmem_t mem_get_flags( vmem_t vaddr ) {
	if( !mem_check_pagetable(vaddr) ) {
		return 0;
	}
	return MEM_PTBL_ADDR(vaddr)[ MEM_PTBL_INDEX(vaddr) ] & 0xFFF;
}

void mem_set_mmu_entry( vmem_t vaddr, page_entry entry ) {
	mem_check_pagetable(vaddr);
	MEM_PTBL_ADDR(vaddr)[ MEM_PTBL_INDEX(vaddr) ] = static_cast<uint64_t>(entry);
}

void mem_set_physical_address( vmem_t vaddr, pmem_t paddr ) {
	mem_check_pagetable(vaddr);
	uint16_t flags = (MEM_PTBL_ADDR(vaddr)[ MEM_PTBL_INDEX(vaddr) ]) & 0xFFF;
	MEM_PTBL_ADDR(vaddr)[ MEM_PTBL_INDEX(vaddr) ] = (paddr & ~(0xFFF)) | flags;
}

void mem_set_flags( vmem_t vaddr, uint16_t flags ) {
	mem_check_pagetable(vaddr);
	pmem_t paddr = MEM_PTBL_ADDR(vaddr)[ MEM_PTBL_INDEX(vaddr) ] & ~(0xFFF);
	MEM_PTBL_ADDR(vaddr)[ MEM_PTBL_INDEX(vaddr) ] = paddr | (flags & 0xFFF);
}

uint32_t flags_to_x86_64(paging::page_flags flags)
{
	uint32_t ret = 0;
	ret |= ((flags.global ? 1 : 0)		<< 8 );
        ret |= ((flags.page_size ? 1 : 0)	<< 7 );
        ret |= ((flags.dirty ? 1 : 0)		<< 6 );
        ret |= ((flags.accessed ? 1 : 0)	<< 5 );
        ret |= ((flags.cache_disabled ? 1 : 0)	<< 4 );
        ret |= ((flags.write_through ? 1 : 0)	<< 3 );
        ret |= ((flags.user_accessible? 1 : 0)	<< 2 );
        ret |= ((flags.rw_accessible ? 1 : 0) 	<< 1 );
        ret |= ((flags.present ? 1 : 0)	<< 0 );
	return ret;
}

void paging::map_range(
		vmem_t vaddr,
		pmem_t paddr,
		unsigned int n_pages,
		paging::page_flags flags)
{
	uint32_t pflags = flags_to_x86_64(flags);
	for(unsigned int i=0;i<n_pages;i++) {
		if(!mem_check_pagetable(vaddr)) {
			mem_create_pagetable(vaddr);
		}
		MEM_PTBL_ADDR(vaddr)[MEM_PTBL_INDEX(vaddr)] =
			(paddr & ~(0xFFF)) | pflags;
		refresh_table(vaddr);
		vaddr += 0x1000;
		paddr += 0x1000;
	}
}

void paging::unmap_range(vmem_t vaddr, unsigned int n_pages)
{
	for(unsigned int i=0;i<n_pages;i++) {
		if(!mem_check_pagetable(vaddr)) {
			continue;
		}
		MEM_PTBL_ADDR(vaddr)[MEM_PTBL_INDEX(vaddr)] = 0;
		refresh_table(vaddr);
		vaddr += 0x1000;
	}
}

void paging::refresh_table(vmem_t vaddr)
{
	asm volatile("invlpg (%0)" : : "r"(vaddr) : "memory");
}

bool pfaulting = false;

void paging::handle_pagefault(uint64_t errcode, vmem_t fault_addr)
{
	if(fault_addr < 0x1000) {
		terminal_writestring("\npaging: Attempted to access NULL page!");
		while(true) {
			asm volatile("cli\n\t"
					"hlt\n\t" : : : "memory");
		}
	}
	
	terminal_writestring("\npaging: Page fault at 0x");
	terminal_writehex(fault_addr);
	terminal_writestring("\npaging: Faultcode = 0x");
	terminal_writehex(errcode);
	if( pfaulting ) {
		terminal_writestring("\npaging: Recursive PF!");
		while(true) {
			asm volatile("cli\n\t"
					"hlt\n\t" : : : "memory");
		}
	}
	pfaulting = true;
	pmem_t new_page = physical_memory::allocate(1);
	terminal_writestring("\npaging: Allocated paddr 0x");
	terminal_writehex(new_page);
	if(!mem_check_pagetable(fault_addr)) {
		mem_create_pagetable(fault_addr);
	}
	MEM_PTBL_ADDR(fault_addr)[MEM_PTBL_INDEX(fault_addr)] =
		(new_page & ~(0xFFF)) | 0x1;
	refresh_table(fault_addr);
	pfaulting = false;
}
