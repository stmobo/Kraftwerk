// Interface to virtual memory subsystem
#pragma once
#include "interface/types.h"

namespace virtual_memory {

// Higher half memory start address.
#define HH_START 0xFFFF800000000000

// vmem linked-list node
struct vmem_ll_node {
	vmem_t alloc_st;	// alloc start
	size_t size;		// size in pages
	bool free;		// free or not
	vmem_ll_node* next;	// linked list pointer
};	// size (3*8)+1 = 25 (order 1)

// vmem linked-list region descriptor
struct vmem_ll_reg_desc {
	vmem_t reg_start;	// region start
	size_t reg_size;	// region size in pages
	vmem_ll_node* list;	// linked list start pointer
};	// size (3*8) = 24 (order 1)

void initialize();

// allocate from main kernel vmem area
vmem_t allocate_kern(unsigned int n_pages);
bool reserve_kern(vmem_t where, unsigned int n_pages);
void free_kern(vmem_t where);
bool is_allocated_kern(vmem_t where, unsigned int n_pages);

// allocate from given region
vmem_t allocate_region(vmem_ll_reg_desc reg, unsigned int n_pages);
bool reserve_region(vmem_ll_reg_desc reg, vmem_t where, unsigned int n_pages);
void free_region(vmem_ll_reg_desc reg, vmem_t where);
bool is_allocated_region(
	vmem_ll_reg_desc reg,
	vmem_t where,
	unsigned int n_pages
);

// manipulate page-tables
bool map(vmem_t v_where, pmem_t p_where, unsigned int n_pages);
bool unmap(vmem_t where, unsigned int n_pages);


};
