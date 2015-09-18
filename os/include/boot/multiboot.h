#pragma once
#include "interface/types.h"

struct multiboot_info {
	uint32_t flags;
	
	// controlled by flags, bit 0
	uint32_t mem_lower;	// amount of lowmem (<=640KiB)
	uint32_t mem_upper;	// amount of himem (everything else)
	
	// controlled by flags bit 1
	uint32_t boot_device;	// boot drive and partitions
	
	// controlled by flags bit 2
	uint32_t cmdline;	// address to commandline
	
	// controlled by flags bit 3
	uint32_t mods_count;	// Number of modules
	uint32_t mods_addr;	// Address to module structure
	
	// controlled by flags bit 4 and/or 5
	multiboot_syms_info syms;
	
	// controlled by flags bit 6
	uint32_t mmap_length;
	uint32_t mmap_addr;
	
	// controlled by flags bit 7
	uint32_t drives_length;
	uint32_t drives_addr;
	
	// controlled by flags bit 8
	uint32_t config_table;
	
	// controlled by flags bit 9
	uint32_t boot_loader_name;
	
	// controlled by flags bit 10
	uint32_t apm_table;
	
	// controlled by flags bit 11
	uint32_t vbe_control_info;
	uint32_t vbe_mode_info;
	uint16_t vbe_mode;
	uint16_t vbe_interface_seg;
	uint16_t vbe_interface_off;
	uint16_t vbe_interface_len;
} __attribute__((packed));

union multiboot_syms_info {
	struct aout_syms {
		uint32_t tabsize;
		uint32_t strsize;
		uint32_t addr;
		uint32_t reserved;
	} __attribute__((packed)) aout_syms_info;
	struct elf_syms {
		uint32_t num;
		uint32_t size;
		uint32_t addr;
		uint32_t shndx;
	} __attribute__((packed)) elf_syms_info;
};

struct multiboot_mmap_t {
	uint32_t size;
	uint64_t base_addr;
	uint64_t length;
	uint32_t type;
} __attribute__((packed));