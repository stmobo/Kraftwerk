// Static pool-based memory allocator
// for use when the usual memory allocation methods are unavailable
// (for example, during early boot and memory init)

template< size_t PoolSize >
class static_mem_pool {
    uint8_t pool[PoolSize];

    void* alloc( size_t num_bytes );
    void free( void* ptr );
};

void* static_mem_pool::alloc( size_t num_bytes ) {
    
}
