#pragma once
#include "interface/types.h"

extern void*	kmalloc(size_t n_bytes, unsigned int flags=0);
extern void	kfree(void* allocation);
extern void	malloc_init();
#define MALLOC_INIT_PAGES	32
#define MALLOC_EMERG_PAGES	32
#define MFLAGS_EMERG		1