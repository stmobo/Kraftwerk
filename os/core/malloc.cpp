#include "core/malloc.h"

/*
 * Allocator design:
 *  ) Pages are divided to fit power of two "chunks".
 *  ) These powers of two range from 4 to 10 (16 bytes to 1024 bytes)
 *  ) A given page only allocates chunks of one type.
 *  ) Allocations greater than 1024 bytes use the mmap allocator.
 *  ) Chunks are tracked with a bitmap at page start; for a 16-byte chunk
 *  page, this will require 32 bytes of overhead, or two chunks out of 256.
 *  ) An extra chunk is reserved for misc. information, bringing the total no.
 * of reserved space per page to 48 bytes, or 3 chunks for a 16-byte chunk page.
 */

struct malloc_page {
void* page_start;           // 8 bytes
unsigned int n_allocations; // 8 bytes
} __attribute__((packed)); // size: chunk-0

struct malloc_page_header {
uint64_t     page_bitmap[4];  // 8 bytes * 4 = 32 bytes
uint8_t      page_chunk_size; // 1 bytes
malloc_page* page_descriptor; // 8 bytes
uint8_t      reserved[7];     // 48 - 41 = 7 bytes
} __attribute__((packed));

malloc_page* malloc_pages[7]; // (10 - 4) + 1

vmem_t get_chunk_address(
    vmem_t page_start, unsigned int chunk_n, unsigned int order ) {
    
    //return page_start + ( chunk_n * (2<<(order+4)) );
    return page_start + ( chunk_n << (order+4) ) );
}

unsigned int get_alloc_order( vmem_t alloc_start ) {
    vmem_t page_start = alloc_start & ~(0xFFF);
    malloc_page_header* header =
        reinterpret_cast<malloc_page_header*>(page_start);

    return header->page_chunk_size;
}

malloc_page* malloc_prepare_new_page( unsigned int order ) {
    // chunk  1 must always be marked for all pages
    // chunks 1 & 2 must both be marked for an order-1 page
    // chunks 1 2 & 3 must all be marked for an order-0 page
    
    vmem_t new_page = virtual_memory::allocate(1);
    malloc_page_header* page_header =
        reinterpret_cast<malloc_page_header*>(new_page);
        
    if( order == 0 ) {
        page_header->page_bitmap[0] |= 7; // 0b111
    } else if (order == 1) {
        page_header->page_bitmap[0] |= 3; // 0b11
    } else {
        page_header->page_bitmap[0] |= 1; // 0b1
    }
    page_header->page_chunk_size = order;
    
    malloc_page* page_descriptor = NULL;
    
    // allocate 16 bytes
    if( malloc_pages[0] == NULL ) {
        // allocate a new page to hold the malloc page descriptor
        vmem_t new_malloc_page = virtual_memory::allocate(1);
        malloc_page_header* page_header =
            reinterpret_cast<malloc_page_header*>(new_malloc_page);
            
        page_header->page_bitmap[0] |= 0x1F; // 0b11111
        page_header->page_chunk_size = 0;
        
        malloc_page* new_page_desc = reinterpret_cast<malloc_page*>(
            get_chunk_address( new_malloc_page, 4, 0 ) );
        
        new_page_desc->page_start = reinterpret_cast<void*>(new_malloc_page);
        new_page_desc->n_allocations = 2;
        
        page_descriptor = reinterpret_cast<malloc_page*>(
            get_chunk_address( new_malloc_page, 5, 0 ) );
    } else {
        unsigned int max_alloc = 0;
        malloc_page* max_page = NULL;
        malloc_page* cur = malloc_pages[0];
        
        while( cur != NULL ) {
            // ignore full pages
            if( cur->n_allocations >= 255 ) {
                continue;
            }
            
            if( cur->n_allocations > max_alloc ) {
                max_alloc = cur->n_allocations;
                max_page = cur;
            }
            
            cur = cur->next;
        }

        if( max_page == NULL ) { // all pages are full, allocate a new one
            vmem_t new_malloc_page = virtual_memory::allocate(1);
            malloc_page_header* page_header =
                reinterpret_cast<malloc_page_header*>(new_malloc_page);
                
            page_header->page_bitmap[0] |= 0x1F;
            page_header->page_chunk_size = 0;
            
            malloc_page* new_page_desc = reinterpret_cast<malloc_page*>(
                get_chunk_address( new_malloc_page, 4, 0 ) );
            
            new_page_desc->page_start = reinterpret_cast<void*>(new_malloc_page);
            new_page_desc->n_allocations = 2;
            
            page_descriptor = reinterpret_cast<malloc_page*>(
                get_chunk_address( new_malloc_page, 5, 0 ) );
        } else {
            malloc_page_header* page =
                reinterpret_cast<malloc_page_header*>(max_page->page_start);
            unsigned int slot = 0;
            
            for(unsigned int i=0;i<4;i++) {
                if( page->page_bitmap[i] != ~0 ) {
                    for(unsigned int i2=0;i2<64;i2++) {
                        if( !(page->page_bitmap[i] & (1<<i2)) ) {
                            slot = (i*64) + i2;
                            page->page_bitmap[i] |= (1<<i2);
                        }
                    }
                }
            }
            max_page->n_allocations++;
            
            page_descriptor = reinterpret_cast<malloc_page*>(
                get_chunk_address( 
                    reinterpret_cast<vmem_t>(max_page->page_start), slot, 0 ) );
        }
    }
    
    page_descriptor->page_start = reinterpret_cast<void*>( new_page );
    page_descriptor->n_allocations = 0;
    
    return page_descriptor;
}

void* kmalloc( size_t bytes ) {
    if( bytes > 1024 ) { // go to mmap
        return mmap( bytes );
    } // order must be <= 10

    unsigned int order = 0;
    unsigned int tmp = bytes;
    while( tmp >>= 1 ) {
        order++;
    }

    if( order < 4 ) {
        order = 4;
    }
    order -= 4; // order 0 = 2^4 (16) bytes, order 1 = 2^5 (32) bytes, etc...
    
    // Place new allocations in the most used page so far, ignoring full pages.
    // This tends to "clump" together allocations and makes garbage collection
    // of unused pages easier.

    // possibly keep the page descriptors in a heap structure to
    // make this faster than iterating over every page for a given order?
    unsigned int max_alloc = 0;
    malloc_page* max_page = NULL;
    
    if( malloc_pages[order] == NULL ) {
        max_page = malloc_prepare_new_page(order);
    } else {
        malloc_page* cur = malloc_pages[order];
        while( cur != NULL ) {
            // ignore full pages
            if( cur->n_allocations >= (( 4096 >> (order+4) )-1) ) {
                continue;
            }
            
            if( cur->n_allocations > max_alloc ) {
                max_alloc = cur->n_allocations;
                max_page = cur;
            }
            
            cur = cur->next;
        }

        if( max_page == NULL ) { // all pages are full
            max_page = malloc_prepare_new_page(order);
        }
    }
    
    max_page->n_allocations++;
    malloc_page_header* page =
        reinterpret_cast<malloc_page_header*>(max_page->page_start);
        
    unsigned int slot = 0;
    if( order == 0 ) { // use all 4 uint64_t's as bitmasks
        for(unsigned int i=0;i<4;i++) {
            if( page->page_bitmap[i] != ~0 ) {
                for(unsigned int i2=0;i2<64;i2++) {
                    if( !(page->page_bitmap[i] & (1<<i2)) ) {
                        slot = (i*64) + i2;
                        page->page_bitmap[i] |= (1<<i2);
                    }
                }
            }
        }
    } else if (order == 1) { // use 2 uint64_t's for bitmasks
        for(unsigned int i=0;i<2;i++) {
            if( page->page_bitmap[i] != ~0 ) {
                if( page->page_bitmap[i] != ~0 ) {
                    for(unsigned int i2=0;i2<64;i2++) {
                        if( !(page->page_bitmap[i] & (1<<i2)) ) {
                            slot = (i*64) + i2;
                            page->page_bitmap[i] |= (1<<i2);
                        }
                    }
                }
            }
        }
    } else { // use 1 uint64_t for bitmasks
        unsigned int n_bits = (4096>>(order+4));
        for(unsigned int i2=0;i2<n_bits;i2++) {
            if( !(page->page_bitmap[0] & (1<<i2)) ) {
                slot = i2;
                page->page_bitmap[0] |= (1<<i2);
            }
        }
    }
    
    return reinterpret_cast<void*>(
        get_chunk_address(
            reinterpret_cast<uintptr_t>(max_page->page_start), slot, order )
    );
}

void kfree( void* allocation ) {
    vmem_t page_start = reinterpret_cast<uint64_t>(allocation) & ~(0xFFF);
    malloc_page_header* page_header = reinterpret_cast<malloc_page_header*>(
        page_start);
    
    unsigned int slot =
        (reinterpret_cast<uint64_t>(allocation) & 0x3FF) >>
            (page_header->page_chunk_size+4);
    
    if(
        (page_header->page_chunk_size == 0) ||
        (page_header->page_chunk_size == 1) ) {
        
        unsigned int bitmask_slot = slot >> 6; // slot / 64
        unsigned int bitmask_pos = slot & 63; // slot % 64
        
        page_header->page_bitmap[bitmask_slot] &= ~(1<<bitmask_pos);
    } else {
        page_header->page_bitmap[0] &= ~(1<<slot);
    }
    
    page_header->page_descriptor->n_allocations--;
}