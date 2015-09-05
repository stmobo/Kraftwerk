// Interface for dynamic memory / malloc subsystem
#pragma once
#include "interface/types.h"

namespace dynamic_memory {

void* kmalloc( size_t n_bytes );
void  kfree( void* ptr );

void change_allocator( standalone_std::allocator& allocator_obj );

};
