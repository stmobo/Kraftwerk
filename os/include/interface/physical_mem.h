// Interface to physical memory management subsystem
#pragma once
#include "interface/types.h"

namespace physical_memory {

struct boot_mmap_t {
	pmem_t beginning;
	size_t size;
};

const size_t page_size = 0x1000;
#define ADDR_TO_PFN(addr) (addr>>12)

const unsigned int block_cache_size = 20;
#define ALLOC_MAX_ORDER 18

pmem_t allocate( unsigned int n_pages );
bool reserve( pmem_t where, unsigned int n_pages );
void free( pmem_t where, unsigned int n_pages );
bool is_allocated( pmem_t where, unsigned int n_pages );
void initialize(boot_mmap_t* mem_map, unsigned int n_entries);

};
