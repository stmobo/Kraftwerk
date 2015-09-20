// Allocator.h
#include "utility.h"

namespace standalone_std {

template<typename T>
class allocator {
public:
    typedef T value_type;
    typedef T* pointer;
    typedef T& reference;
    typedef const T* const_pointer;
    typedef const T& const_reference;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    template<typename T2> struct rebind {
        typedef allocator<T2> other;
    };
    typedef true_type propagate_on_container_move_assignment;

    virtual allocator() nothrow {};
    virtual allocator(const allocator& alloc) nothrow {};
    virtual template<typename U> allocator(const allocator<U>& alloc) nothrow {};
    virtual ~allocator() {};

    virtual pointer address( reference r ) {
        return &r;
    };

    virtual pointer allocate(
        size_type n,
        allocator<void>::const_pointer hint=0 )
        {
        pointer tmp = new value_type[n];
        return tmp;
    };

    virtual void deallocate( pointer p, size_type n ) {
        delete p;
    };

    virtual size_type max_size() const noexcept {
        return SIZE_MAX;
    };

    template<typename U, typename... ctor_args>
    virtual void construct(U* ptr, ctor_args&&... args) {
        new ((void*)ptr) (forward<ctor_args>(args)...);
    };

    template<typename U>
    virtual void destruct( U* ptr ) {
        ptr->~U();
    };
};

template<> class allocator<void> {
public:
    typedef void* pointer;
    typedef const void* const_pointer;
    typedef void value_type;
    template <typename T2> struct rebind { typedef allocator<T2> other; };
};

template<typename U>
class default_deleter {
public:
    void operator()(U* ptr) { delete ptr; };
};

};
