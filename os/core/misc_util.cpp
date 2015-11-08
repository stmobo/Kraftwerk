#include "core/misc_util.h"

void* memcpy(void* destination, const void* source, size_t length)
{
	uint8_t* d_byteptr = reinterpret_cast<uint8_t*>(destination);
	uint8_t* s_byteptr = reinterpret_cast<uint8_t*>(source);
	for(unsigned int i=0;i<length;i++) {
		d_byteptr[i] = s_byteptr[i];
	}
}