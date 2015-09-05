#pragma once
#include <stdint.h>

struct page_entry {
    uintptr_t address;
    bool global;
    bool page_size; // only used in PD entries (and above?)
    bool dirty;     // only used in PT entries
    bool accessed;
    bool cache_disabled;
    bool write_through;
    bool user_accessible;
    bool rw_accessible;
    bool present;

    page_entry(uint64_t raw_value) {
        address         = raw_value & ~(0xFFF);
        present         = (raw_value & (1<<0));
        rw_accessible   = (raw_value & (1<<1));
        user_accessible = (raw_value & (1<<2));
        write_through   = (raw_value & (1<<3));
        cache_disabled  = (raw_value & (1<<4));
        accessed        = (raw_value & (1<<5));
        dirty           = (raw_value & (1<<6));
        page_size       = (raw_value & (1<<7));
        global          = (raw_value & (1<<8));
    };

    explicit operator uint64_t() {
        uint64_t ret = (address & ~(0xFFF));
        ret |= ( ( global ? 1 : 0 ) << 8 );
        ret |= ( ( page_size ? 1 : 0 ) << 7 );
        ret |= ( ( dirty ? 1 : 0 ) << 6 );
        ret |= ( ( accessed ? 1 : 0 ) << 5 );
        ret |= ( ( cache_disabled ? 1 : 0 ) << 4 );
        ret |= ( ( write_through ? 1 : 0 ) << 3 );
        ret |= ( ( user_accessible ? 1 : 0 ) << 2 );
        ret |= ( ( rw_accessible ? 1 : 0 ) << 1 );
        ret |= ( ( present ? 1 : 0 ) << 0 );
        return ret;
    }
};

/* a 64-bit address takes the following form:
 * bits 0-11:  page bits
 * bits 12-20: page table bits / index
 * bits 21-29: page directory bits / index
 * bits 30-38: page directory pointer table bits / index
 * bits 39-47: page map level 4 bits / index
 * bits 48-63: unused (equal to bit 47)
 *
 * The PML4T as a whole covers the whole 256 TiB address space.
 * Each PML4T entry (or PDPT) covers 512GiB of address space.
 * Each PDPT entry (or page directory) covers 1 GiB of address space.
 * Each PD entry (or page table) covers 2 MiB of address space.
 * And of course, each PT entry (or page) covers 4 KiB of address space.
 */

#define MEM_PML4_INDEX(vaddr) ( (((uint64_t)(vaddr)) >> 39) & 0x1FF )
#define MEM_PDPT_INDEX(vaddr) ( (((uint64_t)(vaddr)) >> 30) & 0x1FF )
#define MEM_PDIR_INDEX(vaddr) ( (((uint64_t)(vaddr)) >> 21) & 0x1FF )
#define MEM_PTBL_INDEX(vaddr) ( (((uint64_t)(vaddr)) >> 12) & 0x1FF )

/* In the PML4T, slot 511 (the last, topmost slot) is taken up by kernel code.
 * Therefore, to avoid overwriting kernel code, we use slot 510 for all of our
 * recursive trickery.
 */
#define MEM_RECURSIVE_SLOT ((uint64_t)510)
#define MEM_RECURSIVE_PTBLS ((0xFFFF000000000000UL) | (MEM_RECURSIVE_SLOT<<39))
#define MEM_RECURSIVE_PDIRS ( MEM_RECURSIVE_PTBLS | (MEM_RECURSIVE_SLOT<<30) )
#define MEM_RECURSIVE_PDPTS ( MEM_RECURSIVE_PDIRS | (MEM_RECURSIVE_SLOT<<21) )
#define MEM_RECURSIVE_PML4T ( MEM_RECURSIVE_PDPTS | (MEM_RECURSIVE_SLOT<<12) )

#define MEM_PML4T_ADDR ((uint64_t*)MEM_RECURSIVE_PML4T)
/* take bits 30-38 (PDPT index -- shr 30), multply by 8 (shl 3), and mask
 * same for below (but we take more bits)
 * These macros get the address of the start of the specified table for a given
 * virtual address; for example, MEM_PTBL_ADDR(0x200) gets the page table covering
 * address 0x200 -- in this case, page table 0.
 */
#define MEM_PDPT_ADDR(vaddr) ((uint64_t*)(MEM_RECURSIVE_PDPTS | ((vaddr >> 27) & 0x00001FF000)))
#define MEM_PDIR_ADDR(vaddr) ((uint64_t*)(MEM_RECURSIVE_PDIRS | ((vaddr >> 18) & 0x003FFFF000)))
#define MEM_PTBL_ADDR(vaddr) ((uint64_t*)(MEM_RECURSIVE_PTBLS | ((vaddr >> 9)  & 0x7FFFFFF000)))
