#pragma once
#include "interface/types.h"

extern void*	kmalloc( size_t n_bytes, unsigned int flags );
extern void	kfree( void* allocation );
extern void	malloc_init();
#define MALLOC_INIT_PAGES	128
#define MALLOC_EMERG_PAGES	128
#define MFLAG_EMERG		1