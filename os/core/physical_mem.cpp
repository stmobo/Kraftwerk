#include "arch/x86-64/page_manip.h"
#include "interface/types.h"
#include "interface/physical_mem.h"

struct pmem_block { // buddy alloc block
	pmem_t block_start;
	pmem_block* next;
};

pmem_block* free_lists[ALLOC_MAX_ORDER+1];
pmem_block* outstanding_allocations[ALLOC_MAX_ORDER];

unsigned int get_alloc_order( unsigned int n_frames ) {
	unsigned int order = 0;
	for(unsigned int i=0;i<=BUDDY_MAX_ORDER;i++) {
		if( (1<<i) == n_frames ) {
			order = i;
			break;
		} else if( ( (1<<i) < n_frames) && ((1<<(i+1)) > n_frames)  ) {
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
		// out of memory scenario
		// panic here?
		return;
	}
	
	fill_free_list(order+1);
	pmem_block* old_block = get_block_from_stack(order+1);
	
	// split it into two blocks
	pmem_block* new_block = (pmem_block*)kmalloc(sizeof(pmem_block));
	
	new_block->block_start = (old_block->block_start) + (1<<(order));
	new_block->next = old_block;
	
	old_block->next = NULL;
	
	free_lists[order] = new_block;
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
	
	pmem_blk* cur = free_lists[order];
	pmem_blk* prev = NULL;
	bool found = false;
	while( cur != NULL ) {
		if( (cur->block_start  == (blk->block_start ^ (1<<order))) ) {
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
void decompose_block( pmem_blk* block, unsigned int start_order, unsigned int end_order ) {
	if( end_order > start_order ) {
		return;
	}
	
	if( start_order == end_order ) {
		block->next = free_lists[start_order];
		free_lists[start_order] = block;
		return;
	}
	
	pmem_block* new_block = (pmem_block*)kmalloc(sizeof(pmem_block));
	new_block->block_start = align_block( block->block_start, start_order-1 ) ^ (1<<start_order);
	
	block->block_start = align_block( block->block_start, start_order-1 );
	
	decompose_block( block, start_order-1, end_order );
	return decompose_block( new_block, start_order-1, end_order );
}

pmem_t physical_memory::allocate( unsigned int n_pages ) {
	if( n_pages > (1<<ALLOC_MAX_ORDER) ) {
		return NULL; // too many pages
	}
	
	unsigned int order = get_alloc_order(n_pages);
	fill_free_list( order );
	
	pmem_block* blk = get_block_from_stack( order );
	pmem_t ret = blk->block_start;
	
	kfree(blk);
	
	return ret;
}

void physical_memory::free( pmem_t where, unsigned int n_pages ) {
	unsigned int order = get_alloc_order(n_pages);
	
	pmem_block* new_block = (pmem_block*)kmalloc(sizeof(pmem_block));
	new_block->block_start = where;
	
	return add_free_list(new_block, order);
}

bool physical_memory::is_allocated( pmem_t where, unsigned int n_pages ) {
	unsigned int order = get_alloc_order(n_pages);
	
	pmem_t block_start = align_block( where, order );
	
	pmem_block* cur = free_lists[order];
	
	if( cur != NULL ) {
		while( cur != NULL ) {
			if( cur->block_start == tmp_blk_start ) {
				return false;
			} else {
				cur = cur->next;
			}
		}
	}
	
	bool found_block = false;
	unsigned int split_order = order;
	
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
	
	if( cur != NULL ) {
		while( cur != NULL ) {
			if( cur->block_start == tmp_blk_start ) {
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
	
	decompose_block( cur, split_order, order );
	
	cur = free_lists[order];
	prev = NULL;
	found_block = false;
	
	while( cur != NULL ) {
		if( cur->block_start == block_start ) {
			if( prev == NULL ) {
				free_lists[order] = cur->next;
			} else {
				prev->next = cur->next;
			}
			kfree(cur);
			break;
		} else {
			prev = cur;
			cur = cur->next;
		}
	}
	
	return true;
}

/* Initialize the physical memory manager.
*/
void physical_memory::initialize(boot_mmap_t* mem_map, unsigned int n_entries)
{
	for(unsigned int i=0;i<n_entries;i++) {
		unsigned int n_blocks = (mem_map[i].size>>12)>>ALLOC_MAX_ORDER;
		n_blocks++;
		for(unsigned int j=0;j<n_blocks;j++) {
			pmem_block* nblock = kmalloc(sizeof(*nblock));
			nblock->start = mem_map[i].beginning +
				(j<<(ALLOC_MAX_ORDER+12));
			nblock->next = free_lists[ALLOC_MAX_ORDER];
			free_lists[ALLOC_MAX_ORDER] = nblock;
		}
	}
}