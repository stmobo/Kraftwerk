#include "interface/types.h"

namespace paging {
	struct page_flags {
		bool global;
		bool page_size; // only used in PD entries (and above?)
		bool dirty;     // only used in PT entries
		bool accessed;
		bool cache_disabled;
		bool write_through;
		bool user_accessible;
		bool rw_accessible;
		bool present;
	};

	void map_range(
		vmem_t vaddr,
		pmem_t paddr,
		unsigned int n_pages,
		page_flags flags);
	void unmap_range(vmem_t vaddr, unsigned int n_pages);
	pmem_t get_mapping(vmem_t vaddr);
	void refresh_table(vmem_t vaddr); // see: invalidate_tlb()
	void handle_pagefault(uint64_t errcode, vmem_t fault_addr);
};