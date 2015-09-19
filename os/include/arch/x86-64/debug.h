#pragma once
#include "interface/types.h"

inline uintptr_t get_return_address()
{
	uintptr_t ret;
	asm("movq -8(%%ebp), %0" : "=r"(ret));
	return ret;
}