// Interface to physical memory management subsystem
#pragma once
#include "interface/types.h"

namespace physical_memory {

const size_t page_size = 0x1000;

pmem_t allocate( unsigned int n_pages );
bool reserve( pmem_t where, unsigned int n_pages );
void free( pmem_t where, unsigned int n_pages );
bool is_allocated( pmem_t where, unsigned int n_pages );

};
