// Interface to physical memory management subsystem
#pragma once
#include "interface/types.h"

namespace physical_memory {

const size_t page_size = 0x1000;
#define ADDR_TO_PFN(addr) (addr>>12)

const unsigned int block_cache_size 20;
#define ALLOC_MAX_ORDER 12

pmem_t allocate( unsigned int n_pages );
bool reserve( pmem_t where, unsigned int n_pages );
void free( pmem_t where, unsigned int n_pages );
bool is_allocated( pmem_t where, unsigned int n_pages );
pmem_t align_block( pmem_t in_addr, unsigned int order );

};
