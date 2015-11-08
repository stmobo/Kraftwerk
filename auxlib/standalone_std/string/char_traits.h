#pragma once
#include <stddef.h>
#include <stdint.h>

namespace standalone_std {

template<typename charT>
struct char_traits {
	
};

template<> struct char_traits<char> {
	typedef char	char_type;
	typedef int	int_type;
	typedef size_t	off_type;
	typedef size_t	pos_type;
	
	
}

};