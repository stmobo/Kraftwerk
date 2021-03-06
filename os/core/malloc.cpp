#include "interface/malloc.h"
#include "interface/virtual_mem.h"
#include "arch/x86-64/debug.h"
#include "device/vga.h"

/*
 * Allocator design:
 *  ) Pages are divided to fit power of two "chunks".
 *  ) These powers of two range from 4 to 10 (16 bytes to 1024 bytes)
 *  ) A given page only allocates chunks of one type.
 *  ) Allocations greater than 1024 bytes allocate by on page scales.
 *  ) Chunks are tracked with a bitmap at page start; for a 16-byte chunk
 *  page, this will require 32 bytes of overhead, or two chunks out of 256.
 *  ) An extra chunk is reserved for misc. information, bringing the total no.
 * of reserved space per page to 48 bytes, or 3 chunks for a 16-byte chunk page.
 */

struct malloc_page_header;
 
struct malloc_page {
	malloc_page_header* page_data;	// 8 bytes
	malloc_page* next;		// 8 bytes
} __attribute__((packed)); // size: chunk-0

struct malloc_page_header {
	uint64_t     	page_bitmap[4];	 // 8 bytes * 4 = 32 bytes
	uint8_t     	page_chunk_size; // 1 bytes
	malloc_page*	page_descriptor; // 8 bytes
	unsigned short	n_allocations;	 // 2 bytes	
	uint8_t      	reserved[5];     // 48 - 43 = 5 bytes
} __attribute__((packed));

malloc_page*	malloc_pages[7];	// (10 - 4) + 1
malloc_page*    emerg_pages;		// Emergency store of pages
uint8_t		malloc_init_space[MALLOC_INIT_PAGES*0x1000] __attribute__((aligned (0x1000)));
uint8_t		malloc_emerg_space[MALLOC_EMERG_PAGES*0x1000] __attribute__((aligned (0x1000)));

vmem_t get_chunk_address(
	vmem_t page_start, unsigned int chunk_n, unsigned int order)
{
	
	//return page_start + ( chunk_n * (2<<(order+4)) );
	return page_start + ( chunk_n << (order+4) );
}

unsigned int get_alloc_order(vmem_t alloc_start)
{
	vmem_t page_start = alloc_start & ~(0xFFF);
	malloc_page_header* header =
		reinterpret_cast<malloc_page_header*>(page_start);

	return header->page_chunk_size;
}

void malloc_prepare_pghdr(
	malloc_page* desc,
	malloc_page_header* page_header,
	unsigned int order)
{
	if( order == 0 ) {
		page_header->page_bitmap[0] |= 7; // 0b111
	} else if (order == 1) {
		page_header->page_bitmap[0] |= 3; // 0b11
	} else {
		page_header->page_bitmap[0] |= 1; // 0b1
	}
	page_header->page_chunk_size = order;
	page_header->n_allocations = 0;
	page_header->page_descriptor = desc;
}

/*
	Allocate a page to hold a new descriptor.
	Used in malloc_allocate_new_page().
*/
malloc_page* malloc_allocate_descriptor_page()
{
	vmem_t new_malloc_page = virtual_memory::allocate_kern(1);
	malloc_page_header* page_header =
		reinterpret_cast<malloc_page_header*>(new_malloc_page);
	
	malloc_page* new_page_desc = reinterpret_cast<malloc_page*>(
		get_chunk_address(new_malloc_page, 4, 0));
	
	new_page_desc->page_data = page_header;
	malloc_prepare_pghdr(new_page_desc, page_header, 0);
	
	page_header->page_bitmap[0] |= 0x1F;
	page_header->page_chunk_size = 0;
	page_header->n_allocations = 2;
	
	return reinterpret_cast<malloc_page*>(
		get_chunk_address(new_malloc_page, 5, 0) );
}

unsigned int malloc_mark_open_slot(malloc_page_header* page)
{
	unsigned int order = page->page_chunk_size;
	unsigned int n_slots = (4096>>(order+4));
	for(unsigned int i=0;i<n_slots;i++) {
		// !(page->page_bitmap[i/64] & (1<<(i%64)))
		if( (page->page_bitmap[i>>6] & ( 1<<(i&63) )) == 0 ) {
			page->page_bitmap[i>>6] |= (1<<(i&63));
			return i;
		}
	}
	return 0;
}

/* Finds the first open slot and allocates that.
 * Returns NULL on failure.
 */
void* malloc_direct_alloc(malloc_page_header* page)
{
	unsigned int slot = malloc_mark_open_slot(page);
	if(!slot) {
		return NULL;
	}
	page->n_allocations++;
	return reinterpret_cast<void*>(page+(slot<<(page->page_chunk_size+4)));
}

/* Allocate from emergency page stores.
 */
void* malloc_emerg_alloc(unsigned int order)
{
	malloc_page* cur = emerg_pages;
	
	while(cur != NULL) {
		malloc_page_header* hdr = cur->page_data;
		if(hdr->n_allocations == 0) {
			hdr->page_chunk_size = order;
		}
		if((hdr->page_chunk_size == order) &&
			hdr->n_allocations < (4096>>(order+4))) {
			/*
			void* ret = malloc_direct_alloc(hdr);
			vmem_t new_page =
				virtual_memory::allocate_kern(1);
			malloc_page* new_desc = (malloc_page*)
				kmalloc(sizeof(*new_desc), MFLAGS_EMERG);
			
			new_desc->page_data =
				reinterpret_cast<malloc_page_header*>(new_page);
				
			malloc_prepare_pghdr(
				new_desc, new_desc->page_data, 0);
			
			new_desc->next = emerg_pages;
			emerg_pages = new_desc;
			*/
			
			// if the next allocation in this block is the midway
			// point of the available space in this block...
			if((hdr->n_allocations+1) == (4096>>(order+5))) {
				// ...then allocate a new page.
				vmem_t new_page =
				virtual_memory::allocate_kern(1);
				
				malloc_page* new_desc = (malloc_page*)
				kmalloc(sizeof(*new_desc), MFLAGS_EMERG);
				
				new_desc->page_data =
				reinterpret_cast<malloc_page_header*>(new_page);
					
				malloc_prepare_pghdr(
					new_desc, new_desc->page_data, 0);
				
				new_desc->next = emerg_pages;
				emerg_pages = new_desc;
			}
			return malloc_direct_alloc(hdr);
		}
		cur = cur->next;
	}
	
	return NULL;
}

/* Adds some initial pages to malloc.
 * Splits the provided init-space 50-50 between order-0 and order-1
 * pages. Most of the space should actually be set for order-0 pages,
 * but 50-50's easier for now. Besides, even as it is, it's probably overkill
 * for setting up the p/v-mem managers.
 *
 * Also adds the emergency pages to the store.
 */
void malloc_init()
{
	terminal_writestring("Initializng kernel dynamic allocation.\n");
	
	malloc_page_header* p0 =
		reinterpret_cast<malloc_page_header*>(&(malloc_init_space[0]));
		
	//p0->page_chunk_size = 0;
	
	malloc_prepare_pghdr(NULL, p0, 0);
	
	malloc_page* p0_descriptor = (malloc_page*)malloc_direct_alloc(p0);
	malloc_page* p1_descriptor = (malloc_page*)malloc_direct_alloc(p0);
	
	terminal_writestring("Heap initspace starts at 0x");
	terminal_writehex(reinterpret_cast<uint64_t>(p0));
	terminal_putchar('\n');
	
	p0_descriptor->page_data = reinterpret_cast<malloc_page_header*>(
		malloc_init_space);
	//malloc_prepare_pghdr(p0_descriptor, p0_descriptor->page_data, 0);
	p0->page_descriptor = p0_descriptor;
	p0_descriptor->page_data->n_allocations = 2;
	
	p1_descriptor->page_data = reinterpret_cast<malloc_page_header*>(
		&(malloc_init_space[0x1000]));
	malloc_prepare_pghdr(p1_descriptor, p1_descriptor->page_data, 0);
	
	malloc_page* ord0_head = p0_descriptor;
	malloc_page* ord1_head = p1_descriptor;
	
	//terminal_writestring("\nmalloc: initial p1 descriptor at 0x");
	//terminal_writehex(reinterpret_cast<uintptr_t>(p1_descriptor));
	
	for(unsigned int i=2;i<MALLOC_INIT_PAGES;i+=2) {
		/*
		malloc_page* cur = malloc_pages[0];
		while(cur != NULL) {
			if(cur->page_data->n_allocations<252) {
				break;
			}
			cur = cur->next;
		}
		malloc_page* nd1 =
			(malloc_page*)malloc_direct_alloc(cur->page_data);
		
		malloc_page* nd2 =
			(malloc_page*)malloc_direct_alloc(cur->page_data);
		*/
		
		malloc_page* nd1 =
			(malloc_page*)malloc_direct_alloc(
				p0_descriptor->page_data);
			
		malloc_page* nd2 =
			(malloc_page*)malloc_direct_alloc(
				p0_descriptor->page_data);
		
		//terminal_writestring("\nmalloc: new o0 page descriptor at 0x");
		//terminal_writehex(reinterpret_cast<uintptr_t>(nd1));
		
		//terminal_writestring("\nmalloc: new o1 page descriptor at 0x");
		//terminal_writehex(reinterpret_cast<uintptr_t>(nd1));
		
		nd1->page_data = reinterpret_cast<malloc_page_header*>(
			&(malloc_init_space[i*0x1000]));
		malloc_prepare_pghdr(nd1, nd1->page_data, 0);
		
		nd2->page_data = reinterpret_cast<malloc_page_header*>(
			&(malloc_init_space[(i+1)*0x1000]));
		malloc_prepare_pghdr(nd2, nd2->page_data, 1);
		
		nd1->next = ord0_head;
		nd2->next = ord1_head;
		
		ord0_head = nd1;
		ord1_head = nd2;
	}
	
	malloc_pages[0] = ord0_head;
	malloc_pages[1] = ord1_head;
	
	emerg_pages = NULL;
	malloc_page* emerg_head = NULL;
	for(unsigned int i=0;i<MALLOC_EMERG_PAGES;i++) {
		malloc_page* cur = malloc_pages[0];
		while(cur != NULL) {
			if(cur->page_data->n_allocations<254) {
				break;
			}
			cur = cur->next;
		}
		
		malloc_page* nd1 =
			(malloc_page*)malloc_direct_alloc(cur->page_data);
		nd1->page_data = reinterpret_cast<malloc_page_header*>(
			&(malloc_emerg_space[i*0x1000]));
		malloc_prepare_pghdr(nd1, nd1->page_data, 0);
		
		/*
		terminal_writestring("\nmalloc: new o0 page descriptor at 0x");
		terminal_writehex(reinterpret_cast<uintptr_t>(nd1));
		*/
		
		nd1->next = emerg_head;
		emerg_head = nd1;
	}
	emerg_head = emerg_pages;
	
	terminal_writestring("Kernel dynamic memory initialization complete.\n");
}

/*
	Allocate a new descriptor. Does not use kmalloc().
	Used by malloc_prepare_new_page().
*/
malloc_page* malloc_allocate_descriptor()
{
	unsigned int max_alloc = 0;
	malloc_page* max_page = NULL;
	malloc_page* cur = malloc_pages[0];
	
	while( cur != NULL ) {
		// ignore full pages
		if( cur->page_data->n_allocations >= 252 ) {
			continue;
		}
		
		if( cur->page_data->n_allocations > max_alloc ) {
			max_alloc = cur->page_data->n_allocations;
			max_page = cur;
		}
		
		cur = cur->next;
	}

	if( max_page == NULL ) { // all pages are full, allocate a new one
		return malloc_allocate_descriptor_page();
	} else {
		malloc_page_header* page = max_page->page_data;
		unsigned int slot = malloc_mark_open_slot(page);
		max_page->page_data->n_allocations++;
		
		return reinterpret_cast<malloc_page*>(
			get_chunk_address( 
				reinterpret_cast<vmem_t>(max_page->page_data), slot, 0 ) );
	}
}

malloc_page* malloc_prepare_new_page( unsigned int order ) {
	// chunk  1 must always be marked for all pages
	// chunks 1 & 2 must both be marked for an order-1 page
	// chunks 1 2 & 3 must all be marked for an order-0 page
	
	vmem_t new_page = virtual_memory::allocate_kern(1);
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
	
	page_descriptor->page_data =
		reinterpret_cast<malloc_page_header*>(new_page);
	page_descriptor->page_data->n_allocations = 0;
	
	return page_descriptor;
}

void* kmalloc( size_t bytes, unsigned int flags )
{
	/*
	terminal_writestring("\nmalloc: attempt allocation, size=0x");
	terminal_writehex(bytes);
	terminal_writestring("\nmalloc: flags=0x");
	terminal_writehex(flags);
	terminal_writestring("\nmalloc: caller=0x");
	terminal_writehex(get_return_address());
	*/
	
	if( bytes > 1024 ) { // go to mmap
		return reinterpret_cast<void*>(virtual_memory::allocate_kern(
			((bytes>>12) > 0) ? (bytes>>12) : 1));
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
	
	// allocate first nonempty page we can 
	malloc_page* chosen_page = NULL;
	
	if( malloc_pages[order] == NULL ) {
		if(flags & MFLAGS_EMERG) {
			return malloc_emerg_alloc(order);
		} else {
			chosen_page = malloc_prepare_new_page(order);
		}
	} else {
		malloc_page* cur = malloc_pages[order];
		while( cur != NULL ) {
			// ignore full pages
			/*
			terminal_writestring("\nmalloc: malloc_pages[order] = 0x");
			terminal_writehex(reinterpret_cast<uintptr_t>(&malloc_pages[order]));
			*/
			//terminal_writestring("\nmalloc: cur = 0x");
			//terminal_writehex(reinterpret_cast<uintptr_t>(cur));
			/*
			terminal_writestring("\nmalloc: cur->page_data = 0x");
			terminal_writehex(reinterpret_cast<uintptr_t>(cur->page_data));
			terminal_writestring("\nmalloc: order = 0x");
			terminal_writehex(order);
			terminal_writestring("\nmalloc: n_allocations = 0x");
			terminal_writehex(cur->page_data->n_allocations);
			*/
			
			if(cur->page_data->n_allocations <
			(( 4096 >> (order+4) )-1)) {
				chosen_page = cur;
				break;
			}
			
			cur = cur->next;
		}

		if( chosen_page == NULL ) { // all pages are full
			if(flags & MFLAGS_EMERG) {
				return malloc_emerg_alloc(order);
			} else {
				chosen_page = malloc_prepare_new_page(order);
			}
		}
	}
	
	chosen_page->page_data->n_allocations++;
	
	unsigned int slot = malloc_mark_open_slot(chosen_page->page_data);
	
	return reinterpret_cast<void*>(
		get_chunk_address(
			reinterpret_cast<uintptr_t>(chosen_page->page_data), slot, order )
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
	
	page_header->n_allocations--;
}

void* operator new(size_t allocsz)
{
	return kmalloc(allocsz, 0);
}

void* operator new[](size_t allocsz)
{
	return kmalloc(allocsz, 0);
}

void operator delete(void* ptr)
{
	return kfree(ptr);
}

void operator delete[](void* ptr)
{
	return kfree(ptr);
}

void operator delete(void* ptr, long unsigned int size)
{
	return kfree(ptr);
}

void operator delete[](void* ptr, long unsigned int size)
{
	return kfree(ptr);
}