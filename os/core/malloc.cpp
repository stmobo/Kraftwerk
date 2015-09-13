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

/*
	Allocate a page to hold a new descriptor.
	Used in malloc_allocate_new_page().
*/
malloc_page* malloc_allocate_descriptor_page() {
	vmem_t new_malloc_page = virtual_memory::allocate(1);
	malloc_page_header* page_header =
		reinterpret_cast<malloc_page_header*>(new_malloc_page);
		
	page_header->page_bitmap[0] |= 0x1F;
	page_header->page_chunk_size = 0;
	
	malloc_page* new_page_desc = reinterpret_cast<malloc_page*>(
		get_chunk_address( new_malloc_page, 4, 0 ) );
	
	new_page_desc->page_start = reinterpret_cast<void*>(new_malloc_page);
	new_page_desc->n_allocations = 2;
	
	return reinterpret_cast<malloc_page*>(
		get_chunk_address( new_malloc_page, 5, 0 ) );
}

unsigned int malloc_mark_open_slot(
	malloc_page_header* page, unsigned int order ) {
	
	unsigned int n_slots = (4096>>(order+4));
	for(unsigned int i=0;i<n_slots;i++) {
		if( !(page->page_bitmap[i>>6] & ( 1<<(i&63) )) ) {
			page->page_bitmap[i>>6] |= (1<<(i&63));
			return i;
		}
	}
	return 0;
}

/*
	Allocate a new descriptor. Does not use kmalloc().
	Used by malloc_prepare_new_page().
*/
malloc_page* malloc_allocate_descriptor() {
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
		return malloc_allocate_descriptor_page();
	} else {
		malloc_page_header* page =
			reinterpret_cast<malloc_page_header*>(max_page->page_start);
		unsigned int slot = malloc_mark_open_slot( page, 0 );
		max_page->n_allocations++;
		
		return reinterpret_cast<malloc_page*>(
			get_chunk_address( 
				reinterpret_cast<vmem_t>(max_page->page_start), slot, 0 ) );
	}
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
		page_descriptor = malloc_allocate_descriptor_page();
	} else {
		page_descriptor = malloc_allocate_descriptor();
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
		
	unsigned int slot = malloc_mark_open_slot( page, order );
	
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