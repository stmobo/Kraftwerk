#include "memory/ptr_refcounter.h"

namespace standalone_std {

template<typename T>
class shared_ptr {
    counter* ctr_object;
    allocator allocator;
    T* ptr;
    
    // TODO: implement locking for this!
    void increment() { if(this->ctr_object) { this->ctr_object->strong_count++; } };
    void decrement() {
        if(this->ctr_object) {
            this->ctr_object->strong_count--;
            if(this->ctr_object->strong_count == 0) {
                ctr_object->destroy();
                if(this->ctr_object->weak_count == 0) { // only delete the counter object itself if both the weak and strong counts are zero
                    delete ctr_object;
                }
            }
        }
    };
    
    public:
    
    typedef T element_type;
    
    // constructors
    // default ctor
    shared_ptr() : ctr_object(), ptr(), allocator() {};
    shared_ptr(nullptr_t) : ctr_object(), ptr(), allocator() {};
    
    // from pointer
    template<typename U>
    explicit shared_ptr( U* pt ) : ctr_object( new counter_actual<U, default_deleter>( pt, default_deleter<U>() ) ), ptr(pt), allocator() {};
    
    // with deleter
    template<typename U, typename Del>
    explicit shared_ptr( U* pt, Del deleter ) : ctr_object( new counter_actual<U, Del>(pt, deleter) ), ptr(pt), allocator() {};
    
    // with allocator
    template<typename U, typename Del, typename Alloc>
    explicit shared_ptr(U* pt, Del deleter, Alloc alloc) : ctr_object( new counter_actual<U, default_deleter>( pt, default_deleter<U>() ) ), ptr(pt), allocator(alloc) {};
    
    template<typename Del, typename Alloc>
    explicit shared_ptr(Del deleter, Alloc alloc) : ctr_object( new counter_actual<nullptr_t, default_deleter>( nullptr, default_deleter<U>() ) ), ptr(pt), allocator(alloc) {};
    
    // copy constructors
    shared_ptr( const shared_ptr& rhs ) : ctr_object( rhs.ctr_object ), ptr(rhs.ptr), allocator(rhs.allocator) { increment(); };
    
    template<typename U>
    explicit shared_ptr( const shared_ptr<U>& rhs ) : ctr_object( rhs.ctr_object ), ptr(rhs.ptr), allocator(rhs.allocator) { increment(); };
    
    // copy from weak:
    template<typename U>
    explicit shared_ptr( const weak_ptr<U>& rhs ) : ctr_object( rhs.ctr_object ), ptr(rhs.ptr), allocator() {
        if( this->ctr_object->strong_count > 0 ) {
            increment();
        } else {
            this->ctr_object = nullptr;
            this->ptr = nullptr;
            // maybe throw exception?
        }
    };
    
    // move constructors
    shared_ptr( const shared_ptr&& rhs ) {
        ctr_object = rhs.ctr_object;
        ptr = rhs.ptr;
        allocator = move(rhs.allocator); // only the allocator needs to be moved, the others are pointers which should be able to be copied freely
        
        rhs.ctr_object = nullptr;
        rhs.ptr = nullptr;
        rhs.allocator = allocator();
    };
    
    template<typename U>
    shared_ptr( const shared_ptr<U>&& rhs ) {
        ctr_object = rhs.ctr_object;
        ptr = rhs.ptr;
        allocator = move(rhs.allocator);
        
        rhs.ctr_object = nullptr;
        rhs.ptr = nullptr;
        rhs.allocator = allocator();
    };
    
    // aliasing constructor
    template<typename U>
    shared_ptr( const shared_ptr<U>& rhs, element_type* ptr ) {
        ctr_object = rhs.ctr_object;
        this->ptr = ptr;
        allocator = rhs.allocator;
        
        increment();
    }
    
    // copy assignment
    shared_ptr& operator=(const shared_ptr& rhs ) {
        if( this != &rhs ) {
            decrement();
            this->ctr_object = rhs.ctr_object;
            this->ptr = rhs.ptr;
            increment();
        }
        return *this;
    }
    
    // move assignment
    shared_ptr& operator=(const shared_ptr&& rhs ) {
        if( this != &rhs ) {
            decrement();
            
            this->ctr_object = rhs.ctr_object;
            this->ptr = rhs.ptr;
            
            rhs.ctr_object = nullptr;
            rhs.ptr = nullptr;
        }
        return *this;
    }
    
    // destructor
    ~shared_ptr() { decrement(); };
    
    // member funcs
    void swap(shared_ptr& rhs) {
        shared_ptr tmp = move(*this);
        
        ctr_object = rhs.ctr_object;
        ptr = rhs.ptr;
        allocator = move(rhs.allocator);
        
        rhs = move(tmp);
    };
    
    void reset() {
        decrement();
        this->ctr_object = nullptr;
        this->ptr = nullptr;
    };
    
    element_type* get() {
        return this->ptr;
    };
    
    element_type& operator*() {
        return *(this->ptr);
    };
    
    element_type* operator->() {
        return this->ptr;
    };
    
    unsigned int use_count() {
        if( this->ctr_object ) {
            return this->ctr_object->strong_count;
        } else {
            return 0;
        }
    };
    
    bool unique() {
        if( this->ctr_object ) {
            return ( this->ctr_object->strong_count == 1 );
        }
        return false;
    };
    
    bool operator bool() {
        return (this->ptr == nullptr);
    };
};

};