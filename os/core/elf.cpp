#include "interface/types.h"
#include "interface/malloc.h"
#include "core/misc_util.h"
#include "core/elf.h"

char* elf::lookup_section_name(
	elf64_shdr* sht,
	elf64_half shstrndx,
	elf64_word indx)
{
	if(sht[shstrndx].sh_type != SHT_STRTAB) {
		return NULL;
	}
	
	unsigned int len = strlen(
		reinterpret_cast<char*>(sht[shstrndx].sh_offset+indx));
	
	char* ret = (char*)kmalloc(len+1);
	strcpy(ret, reinterpret_cast<char*>(sht[shstrndx].sh_offset+indx));
	
	return ret;
}

bool section_name_matches(elf64_shdr* sht,
	elf64_half shstrndx,
	elf64_word indx,
	const char* match)
{
	if(sht[shstrndx].sh_type != SHT_STRTAB) {
		return NULL;
	}
	
	char* cur = reinterpret_cast<char*>(sht[shstrndx].sh_offset+indx);
	
	while((*str1 == *str2) && (*str1 != 0) && (*str2 != 0)) {
		if(*cur != *match)
			return false;
	}
	
	if(*cur != *match)
		return false;
	
	return true;
}
