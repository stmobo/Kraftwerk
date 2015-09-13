#pragma once
#include "interface/types.h"

typedef uint64_t elf64_addr;
typedef uint64_t elf64_offset;
typedef uint16_t elf64_half;
typedef uint32_t elf64_word;
typedef int32_t  elf64_sword;
typedef uint64_t elf64_xword;
typedef int64_t  elf64_sxword;

#define EI_NIDENT 16

typedef struct {
    unsigned char e_ident[EI_NIDENT];
    elf64_half    e_type;       // Object type
    elf64_half    e_machine;    // Machine type
    elf64_word    e_version;    // Version type (Must be 1)
    elf64_addr    e_entry;      // Entry point (vaddr)
    elf64_offset     e_phoff;      // Offset to program header table
    elf64_offset     e_shoff;      // Offset to section header table
    elf64_word    e_flags;      // Processor flags
    elf64_half    e_ehsize;     // ELF header size
    elf64_half    e_phentsize;  // Program header table entry size
    elf64_half    e_phnum;      // Number of entries in PHT
    elf64_half    e_shentsize;  // Section header table entry size
    elf64_half    e_shnum;      // Number of entries in SHT
    elf64_half    e_shstrndx;   // Index in SHT of the string table (SHN_UNDEF == no string table)
} elf64_ehdr;

typedef struct {
	elf64_word	sh_name;
	elf64_word	sh_type;
	elf64_xword	sh_flags;
	elf64_addr	sh_addr;
	elf64_offset	sh_offset;
	elf64_xword	sh_size;
	elf64_word	sh_link;
	elf64_word	sh_info;
	elf64_xword	sh_addralign;
	elf64_xword	sh_entsize;
} elf64_shdr;

typedef struct {
	elf64_word  	st_name;
	unsigned char	st_info;
	unsigned char	st_other;
	elf64_half	    st_shndx;
	elf64_addr  	st_value;
	elf64_xword 	st_size;
} elf64_sym;

typedef struct {
	elf64_addr	r_offset;
	elf64_xword	r_info;
} elf64_rel;

typedef struct {
	elf64_addr	    r_offset;
	elf64_xword	    r_info;
	elf64_sxword	r_addend;
} elf64_rela;

#define ELF64_ST_BIND(i)   ((i)>>4)
#define ELF64_ST_TYPE(i)   ((i)&0xf)
#define ELF64_ST_INFO(b,t) (((b)<<4)+((t)&0xf))
#define ELF32_ST_VISIBILITY(o) ((o)&0x3)
#define ELF64_ST_VISIBILITY(o) ((o)&0x3)

typedef struct {
    const char*  name;
    uintptr_t    location;
    unsigned int size;
} module_symbol;