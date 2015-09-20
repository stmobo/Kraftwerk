#pragma once
#include "interface/types.h"

inline uintptr_t get_return_address()
{
	uintptr_t ret;
	asm volatile("movq 8(%%rbp), %0" : "=r"(ret) : : "memory");
	return ret;
}