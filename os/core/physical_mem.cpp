#include "arch/x86-64/page_manip.h"
#include "interface/types.h"
#include "interface/physical_mem.h"
#include "interface/malloc.h"
#include "device/vga.h"

struct pmem_block { // buddy alloc block
	pmem_t block_start;
	pmem_block* next;
};

pmem_block* free_lists[ALLOC_MAX_ORDER+1];
//pmem_block* outstanding_allocations[ALLOC_MAX_ORDER+1];

unsigned int get_alloc_order( unsigned int n_frames ) {
	unsigned int order = 0;
	for(unsigned int i=0;i<=ALLOC_MAX_ORDER;i++) {
		if( (1u<<i) == n_frames ) {
			order = i;
			break;
		} else if( ( (1u<<i) < n_frames) && ((1u<<(i+1)) > n_frames)  ) {
			order = i+1;
			break;
		}
	}
	return order;
}

pmem_t align_block( pmem_t in_addr, unsigned int order ) {
	return in_addr & ~((1<<(order+12))-1);
}

pmem_block* get_block_from_stack( unsigned int order ) {
	pmem_block* ret = free_lists[order];
	free_lists[order] = ret->next;
	
	return ret;
}

void fill_free_list( unsigned int order ) {
	if( free_lists[order] != NULL ) {
		return;
	}
	
	if( order >= ALLOC_MAX_ORDER ) {
		return;
	}
	
	//terminal_writestring("\npmem: filling free list, order = 0x");
	//terminal_writehex(order);
	
	fill_free_list(order+1);
	pmem_block* old_block = get_block_from_stack(order+1);
	
	// split it into two blocks
	pmem_block* new_block = (pmem_block*)kmalloc(
		sizeof(pmem_block), MFLAGS_EMERG);
	
	//terminal_writestring("\npmem: allocated new block at 0x");
	//terminal_writehex(reinterpret_cast<uintptr_t>(new_block));
	
	new_block->block_start = (old_block->block_start) ^ (1<<(order+12));
	new_block->next = old_block;
	
	old_block->next = NULL;
	
	free_lists[order] = new_block;
	
	/*
	terminal_writestring("\npmem: done filling free list, order = 0x");
	terminal_writehex(order);
	*/
}

void add_free_list( pmem_block* blk, unsigned int order ) {
	if( order == ALLOC_MAX_ORDER ) {
		blk->next = free_lists[order];
		free_lists[order] = blk;
		return;
	}
	
	blk->block_start = align_block( blk->block_start, order );

	if( free_lists[order] == NULL ) {
		blk->next = NULL;
		free_lists[order] = blk;
		return;
	}
	
	pmem_block* cur = free_lists[order];
	pmem_block* prev = NULL;
	bool found = false;
	while( cur != NULL ) {
		if( (cur->block_start  == (blk->block_start ^ (1<<(order+12)))) ) {
			found = true;
			break;
		} else {
			prev = cur;
			cur = cur->next;
		}
	}
	
	if( found ) {
		if( prev != NULL ) {
			prev->next = cur->next;
		} else {
			free_lists[order] = cur->next;
		}
		
		kfree(cur);
		
		return add_free_list(blk, order+1);
	} else {
		blk->next = free_lists[order];
		free_lists[order] = blk;
	}
}

// decompose a block of order start_order into blocks of order end_order
void decompose_block( pmem_block* block, unsigned int start_order, unsigned int end_order ) {
	if( end_order > start_order ) {
		return;
	}
	
	if( start_order == end_order ) {
		block->next = free_lists[start_order];
		free_lists[start_order] = block;
		return;
	}
	
	pmem_block* new_block = (pmem_block*)kmalloc(
		sizeof(pmem_block), MFLAGS_EMERG);
	new_block->block_start =
		align_block( block->block_start, start_order-1 )
		^ (1<<(start_order+12-1));
	
	block->block_start = align_block( block->block_start, start_order-1 );
	
	//decompose_block( new_block, start_order-1, end_order );
	new_block->next = free_lists[start_order-1];
	free_lists[start_order-1] = new_block;
	return decompose_block( block, start_order-1, end_order );
}

pmem_t physical_memory::allocate( unsigned int n_pages ) {
	if( n_pages > (1<<ALLOC_MAX_ORDER) ) {
		return NULL; // too many pages
	}
	
	unsigned int order = get_alloc_order(n_pages);
	
	/*
	terminal_writestring("\npmem: Allocating 0x");
	terminal_writehex(n_pages);
	terminal_writestring(" pages, order 0x");
	terminal_writehex(order);
	*/
	
	fill_free_list(order);
	
	pmem_block* blk = get_block_from_stack(order);
	pmem_t ret = blk->block_start;
	
	kfree(blk);
	
	return ret;
}

void physical_memory::free( pmem_t where, unsigned int n_pages ) {
	unsigned int order = get_alloc_order(n_pages);
	
	pmem_block* new_block = (pmem_block*)kmalloc(
		sizeof(pmem_block), MFLAGS_EMERG);
	new_block->block_start = where;
	
	return add_free_list(new_block, order);
}

bool physical_memory::is_allocated( pmem_t where, unsigned int n_pages ) {
	unsigned int order = get_alloc_order(n_pages);
	
	pmem_t block_start = align_block( where, order );
	
	pmem_block* cur = free_lists[order];
	
	if( cur != NULL ) {
		while( cur != NULL ) {
			if( cur->block_start == block_start ) {
				return false;
			} else {
				cur = cur->next;
			}
		}
	}
	
	for( unsigned int i = order+1; i < ALLOC_MAX_ORDER; i++) {
		if( free_lists[i] == NULL ) {
			continue;
		} else {
			pmem_t tmp_blk_start = align_block( where, i );
			
			cur = free_lists[i];
			
			while( cur != NULL ) {
				if( cur->block_start == tmp_blk_start ) {
					return false;
				} else {
					cur = cur->next;
				}
			}
		}
	}
	
	return true;
}

bool physical_memory::reserve( pmem_t where, unsigned int n_pages ) {
	unsigned int order = get_alloc_order(n_pages);
	
	pmem_t block_start = align_block( where, order );
	
	pmem_block* cur = free_lists[order];
	pmem_block* prev = NULL;
	
	bool found_block = false;
	
	/*
	terminal_writestring("\npmem: reserving, address = 0x");
	terminal_writehex(where);
	terminal_writestring("\npmem: order = 0x");
	terminal_writehex(order);
	*/
	
	if( cur != NULL ) {
		while( cur != NULL ) {
			if( cur->block_start == block_start ) {
				found_block = true;
				break;
			} else {
				prev = cur;
				cur = cur->next;
			}
		}
	
		if( found_block ) {
			if( prev == NULL ) {
				free_lists[order] = cur->next;
			} else {
				prev->next = cur->next;
			}
			kfree(cur);
			return true;
		}
	}
	
	found_block = false;
	unsigned int split_order = order;
	
	//terminal_writestring("\npmem: searching for parent block");
	
	for( unsigned int i = order+1; i < ALLOC_MAX_ORDER; i++) {
		if( free_lists[i] == NULL ) {
			continue;
		} else {
			pmem_t tmp_blk_start = align_block( where, i );
			
			cur = free_lists[i];
			prev = NULL;
			
			while( cur != NULL ) {
				if( cur->block_start == tmp_blk_start ) {
					found_block = true;
					split_order = i;
					break;
				} else {
					prev = cur;
					cur = cur->next;
				}
			}
		}
	}
	
	if( found_block == false ) {
		// could not find block to split and no blocks in current free list)
		// cannot allocate
		return false;
	}
	
	//decompose_block( cur, split_order, order );
	for(unsigned int i=split_order;i>=(order+1);i++) {
		pmem_block* new_block = (pmem_block*)kmalloc(
			sizeof(pmem_block), MFLAGS_EMERG);
		new_block->block_start =
			align_block( cur->block_start, i-1 )
			^ (1<<(i-1));
		
		cur->block_start = align_block(cur->block_start, i-1);
		
		if(align_block(where, i-1) == new_block->block_start) {
			// swap new_block and cur
			pmem_block* tmp = new_block;
			new_block = cur;
			cur = tmp;
		}
		
		// cur is the block we want to keep decomposing.
		// new_block is just one we're not looking into further.
		
		new_block->next = free_lists[i-1];
		free_lists[i-1] = new_block;
	}
	
	kfree(cur);
	
	return true;
}

/* Initialize the physical memory manager.
*/
void physical_memory::initialize(boot_mmap_t* mem_map, unsigned int n_entries)
{
	terminal_writestring("\npmem: Initializng kernel pmem allocation.");
	for(unsigned int i=0;i<=ALLOC_MAX_ORDER;i++) {
		free_lists[i] = NULL;
	}
	
	for(unsigned int i=0;i<n_entries;i++) {
		//unsigned int n_blocks = (mem_map[i].size>>12)>>ALLOC_MAX_ORDER;
		unsigned int n_frames = (mem_map[i].size>>12);
		unsigned int order = 0;
		for(unsigned int i2=0;i2<=ALLOC_MAX_ORDER;i2++) {
			if( (1u<<i2) == n_frames ) {
				order = i2;
				break;
			} else if(
				((1u<<i2) < n_frames) &&
				((1u<<(i2+1)) > n_frames)) {
				order = i2;
				break;
			}
		}
		unsigned int n_blocks = n_frames>>order;
		/*
		terminal_writestring("\npmem: Creating block\npmem: order = 0x");
		terminal_writehex(order);
		terminal_writestring("\npmem: beginning = 0x");
		terminal_writehex(mem_map[i].beginning);
		*/
		
		//n_blocks++;
		for(unsigned int j=0;j<n_blocks;j++) {
			pmem_block* nblock =
				(pmem_block*)kmalloc(
					sizeof(*nblock), MFLAGS_EMERG);
			nblock->block_start = mem_map[i].beginning +
				(j<<(order+12));
			nblock->next = free_lists[order];
			free_lists[order] = nblock;
		}
	}
	
	terminal_writestring("\npmem: Finished initializng kernel pmem allocation.");
}