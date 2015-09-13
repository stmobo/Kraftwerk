#pragma once
#include "interface/types.h"

extern void* kmalloc( size_t n_bytes );
extern void  kfree( void* allocation );