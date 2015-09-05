// Interface to virtual memory subsystem
#pragma once
#include "interface/types.h"

namespace virtual_memory {

vmem_t allocate( unsigned int n_pages );
bool reserve( vmem_t where, unsigned int n_pages );
void free( vmem_t where, unsigned int n_pages );
bool is_allocated( vmem_t where, unsigned int n_pages );

bool map( vmem_t v_where, pmem_t p_where, unsigned int n_pages );
bool unmap( vmem_t where, unsigned int n_pages );

};
