#include "interface/types.h"
#include "interface/virtual_mem.h"
#include "interface/malloc.h"
#include "device/vga.h"

virtual_memory::vmem_ll_reg_desc hh_reg; // higher-half region

void virtual_memory::initialize()
{
	terminal_writestring("\nvmem: Initializng kernel vmem allocation.");
	hh_reg.reg_start = HH_START;	// 1st canonical hh address
	hh_reg.reg_size  = 0x800000000;	// 128 TiB
	
	vmem_ll_node* new_node = (vmem_ll_node*)kmalloc(
		sizeof(*new_node), MFLAGS_EMERG);
	
	new_node->alloc_st = hh_reg.reg_start;
	new_node->size = hh_reg.reg_size;
	new_node->free = true;
	new_node->next = NULL;
	
	hh_reg.list = new_node;
	
	// 1st PT of higher-half addresses are mapped to the 1st 2 MiB
	// of physical memory.
	// Or, in equivalent terms, the 1st 512 pages are reserved.
	virtual_memory::reserve_region(hh_reg, HH_START, 512);
	
	terminal_writestring("\nvmem: Finished initializng kernel vmem allocation.");
}

// Split a node into two nodes.
// The "old" node (prev_node) will have its size set to new_size.
// The newly-created node will be marked free, have size new_size and end at
// the old endpoint of prev_node.
virtual_memory::vmem_ll_node* bisect_node(
	virtual_memory::vmem_ll_node* prev_node,
	size_t new_size)
{
	virtual_memory::vmem_ll_node* new_node =
		(virtual_memory::vmem_ll_node*)kmalloc(
			sizeof(*new_node), MFLAGS_EMERG);
	
	new_node->next = prev_node->next;
	prev_node->next = new_node;
	
	new_node->size = prev_node->size - new_size;
	prev_node->size = new_size;
	
	new_node->alloc_st = prev_node->alloc_st + (new_size<<12);
	new_node->free = true;
	
	return new_node;
}

// Search for the first unallocated block of vmem large enough to fit
// and create a new block there
vmem_t virtual_memory::allocate_region(
	vmem_ll_reg_desc reg,
	unsigned int n_pages)
{
	vmem_ll_node* cur = reg.list;
	
	while(cur != NULL) {
		if(!cur->free) {
			continue;
		}
		
		if(cur->size == n_pages) {
			cur->free = false;
			return cur->alloc_st;
		} else if(cur->size > n_pages) {
			vmem_ll_node* nnode = 
				bisect_node(cur, n_pages);
			
			cur->free = false;
			return cur->alloc_st;
		}
		
		cur = cur->next;
	}
	
	return NULL;
}

bool virtual_memory::is_allocated_region(
	vmem_ll_reg_desc reg,
	vmem_t where,
	unsigned int n_pages)
{
	vmem_ll_node* cur = reg.list;
	
	while(cur != NULL) {
		if(!cur->free) {
			continue;
		}
		
		if(
		(cur->alloc_st <= where) &&
		((cur->alloc_st+(cur->size<<12)) >= (where+(n_pages<<12)))
		) {
			return !cur->free;
		}
	}
	
	return true;
}

bool virtual_memory::reserve_region(
	vmem_ll_reg_desc reg,
	vmem_t where,
	unsigned int n_pages)
{
	vmem_ll_node* cur = reg.list;
	
	while(cur != NULL) {
		if(!cur->free) {
			continue;
		}
		
		if(
		(cur->alloc_st <= where) &&
		((cur->alloc_st+(cur->size<<12)) >= (where+(n_pages<<12)))
		) {
			// bisect twice: once to get the beginning, 
			// once to get the end
			if(cur->alloc_st > where) {
				size_t page_diff = (cur->alloc_st-where)>>12;
				cur = bisect_node(
					cur, page_diff);
			}
			
			if(cur->size > n_pages) {
				vmem_ll_node* nn = 
					bisect_node(
						cur, n_pages);
				nn->free = false;
			} else {
				cur->free = false;
			}
			
			return true;
		}
		
		cur = cur->next;
	}
	
	return false;
}

void virtual_memory::free_region(vmem_ll_reg_desc reg, vmem_t where)
{
	vmem_ll_node* cur = reg.list;
	vmem_ll_node* prev = NULL;
	
	while(cur != NULL) {
		if(cur->free) {
			continue;
		}
		
		if(cur->alloc_st == where) {
			if(cur->next->free) {
				cur->next = cur->next->next;
				cur->size += cur->next->size;
				kfree(cur->next);
			}
			if(prev && prev->free) {
				prev->next = cur->next;
				prev->size += cur->size;
				kfree(cur);
			} else {
				cur->free = true;
			}
			return;
		}
		
		prev = cur;
		cur = cur->next;
	}
}

vmem_t virtual_memory::allocate_kern(unsigned int n_pages)
{
	return virtual_memory::allocate_region(hh_reg, n_pages);
}

bool virtual_memory::reserve_kern(vmem_t where, unsigned int n_pages)
{
	return virtual_memory::reserve_region(hh_reg, where, n_pages);
}

void virtual_memory::free_kern(vmem_t where)
{
	return virtual_memory::free_region(hh_reg, where);
}

bool virtual_memory::is_allocated_kern(vmem_t where, unsigned int n_pages)
{
	return virtual_memory::is_allocated_region(hh_reg, where, n_pages);
}