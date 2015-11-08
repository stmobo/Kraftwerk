#pragma once
#include <stddef.h>
#include <stdint.h>

namespace standalone_std {

template<typename T>
class hash {
	typedef size_t	result_type;
	typedef T	argument_type;
	size_t operator()(T hash_input);
};

};