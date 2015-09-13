#include "arch/x86-64/page_manip.h"
#include "interface/types.h"
#include "interface/physical_mem.h"

// Ensures that a page table exists before writing to it:
void mem_create_pagetable( vmem_t vaddr ) {
	// check if the PML4T entry is there
	if( !(MEM_PML4T_ADDR[ MEM_PML4_INDEX(vaddr) ] & 1) ) {
	// not present, make a new PDPT
	pmem_t pdpt = physical_memory::allocate(1);
	MEM_PML4T_ADDR[ MEM_PML4_INDEX(vaddr) ] = (pdpt | 0x101);
	}
	// now it is; check if the PDPT entry is there
	if( !(MEM_PDPT_ADDR(vaddr)[ MEM_PDPT_INDEX(vaddr) ] & 1) ) {
	pmem_t pd = physical_memory::allocate(1);
	MEM_PDPT_ADDR(vaddr)[ MEM_PDPT_INDEX(vaddr) ] = (pd | 0x101);
	}
	// check for PD entry:
	if( !(MEM_PDIR_ADDR(vaddr)[ MEM_PDIR_INDEX(vaddr) ] & 1) ) {
	pmem_t pt = physical_memory::allocate(1);
	MEM_PDIR_ADDR(vaddr)[ MEM_PDIR_INDEX(vaddr) ] = (pt | 0x101);
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
