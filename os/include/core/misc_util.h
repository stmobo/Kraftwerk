#pragma once
#include "interface/types.h"

template<typename char_type>
unsigned int strlen(char_type* str)
{
	unsigned int len=0;
	while(*str++ != 0) {
		len++;
	}
	return len;
}

template<typename char_type>
char* strcpy(char_type* destination, const char_type* source)
{
	while(*source != 0) {
		*destination++ = *source++;
	}
	return destination;
}

template<typename char_type>
char* strncpy(char_type* destination, const char_type* source, size_t num)
{
	for(unsigned int i=0;i<num;i++) {
		destination[i] = source[i];
	}
	return destination;
}


template<typename char_type>
int strcmp(const char_type* str1, const char_type* str2)
{
	//while( (*str1 != 0) && (*str2 != 0) ) {
	while((*str1 == *str2) && (*str1 != 0) && (*str2 != 0)) {
		str1++;
		str2++;
	}
	
	if(*str1 < *str2) {
		return -1;
	} else if(*str1 > *str2) {
		return 1;
	}
	
	return 0;
}

template<typename char_type>
int strncmp(const char_type* str1, const char_type* str2, size_t num)
{
	size_t i = 0;
	while((*str1 == *str2) && (*str1 != 0) && (*str2 != 0) && (i<num) ) {		
		str1++;
		str2++;
		i++;
	}
	
	
	str1--;
	str2--;
	
	if(*str1 < *str2) {
		return -1;
	} else if(*str1 > *str2) {
		return 1;
	}
	
	return 0;
}

char* strcpy(char* destination, const char* source);
void* memcpy(void* destination, const void* source, size_t length);