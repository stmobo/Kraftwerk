namespace standalone_std {

template< typename T, class Alloc = allocator<T> >
class vector {
    Alloc::pointer elements;
    Alloc alloc_object;
    Alloc::size_type object_count;
    Alloc::size_type array_size;

    // reallocate elements as in push/emplace_back
    void reallocate( size_t new_size ) {
        Alloc::pointer tmp = alloc_object.allocate(new_size);
        for(Alloc::size_type i=0;i<this->object_count;i++) {
            *tmp++ = move(this->elements[i]);
        }
        alloc_object.deallocate(this->elements);
        this->array_size = new_size;
        this->elements = tmp;
    }

    // create a "hole" in the vector in front of shift_start for new elements
    void shift( Alloc::pointer shift_start, Alloc::size_type shift_len ) {
        Alloc::difference_type shift_loc = (shift_start - this->elements);
        if( (this->object_count+shift_len) > this->array_size ) {
            Alloc::pointer operate = this->alloc_object.allocate(
                this->object_count+shift_len );

            for(Alloc::size_type i=0;i<=shift_loc;i++) {
                operate[i] = move(this->elements[i]);
            }

            for(Alloc::size_type i=(shift_loc+1);i<this->object_count;i++) {
                operate[i+shift_len] = move(this->elements[i]);
            }

            this->alloc_object.deallocate( this->elements, this->array_size );
            this->elements = operate;
            this->array_size = (this->object_count+shift_len);
        } else {
            for(Alloc::size_type i=(shift_loc+1);i<this->object_count;i++) {
                this->elements[i+shift_len] = move(this->elements[i]);
            }
        }
        this->object_count += shift_len;
    }

public:
    // typedefs:
    typedef T value_type;
    typedef Alloc allocator_type;
    // reference types:
    typedef value_type& reference;
    typedef const value_type& const_reference;
    // pointer + size types:
    typedef Alloc::pointer pointer;
    typedef Alloc::const_pointer const_pointer;
    typedef Alloc::size_type size_type;
    typedef Alloc::difference_type difference_type;
    // iterator types:
    typedef Alloc::pointer iterator;
    typedef Alloc::const_pointer const_iterator;
    typedef Alloc::pointer reverse_iterator;
    typedef Alloc::const_pointer const_reverse_iterator;

    // constructors:
    // TODO: Initializer list support

    // default constructors:
    explicit vector( const Alloc& a ) : alloc_object(a) {};
    vector() : vector( Alloc() ) {};

    // with size and default elements:
    vector( size_type count, const T& value, const Alloc& alloc = Alloc() )
    : alloc_object(alloc) {
        this->elements = alloc_object.allocate( count );
        for(size_type i = 0;i<count;i++) {
            this->elements[i] = value;
        }
    };

    // with size and allocator:
    explicit vector( size_type count, const Alloc& alloc = Alloc() )
    : alloc_object(alloc) {
        this->elements = alloc_object.allocate(count);
    };

    // copy constructor:
    vector( const vector& rhs ) {
        this->alloc_object = rhs.alloc_object;
        this->elements = this->alloc_object.allocate( rhs.array_size );
        for( unsigned int i=0;i<rhs.object_count;i++ ) {
            this->elements[i] = rhs.elements[i];
        }
    };

    // copy ctor with allocator:
    vector( const vector& rhs, const Alloc& alloc )
    : alloc_object(alloc) {
        this->elements = this->alloc_object.allocate( rhs.array_size );
        for( unsigned int i=0;i<rhs.object_count;i++ ) {
            this->elements[i] = rhs.elements[i];
        }
    };

    // move constructor:
    vector( const vector&& rhs ) :
        alloc_object( move(rhs.alloc_object) ),
        elements( rhs.elements ) {
            this->array_size = rhs.array_size;
            this->object_count = rhs.object_count;
            rhs.elements = nullptr;
    };

    // move ctor with allocator:
    vector( const vector&& rhs, const Alloc& alloc ) :
        alloc_object( alloc ),
        elements( rhs.elements ) {
            this->array_size = rhs.array_size;
            this->object_count = rhs.object_count;
            rhs.elements = nullptr;
    };
    // end constructors

    // assignment operators:
    // TODO: honor allocator::propagate_on_container_<copy/move>_assignment

    // copy:
    vector& operator=(const vector& rhs) {
        this->alloc_object = rhs.alloc_object;
        this->elements = this->alloc_object.allocate( rhs.array_size );
        for( unsigned int i=0;i<rhs.object_count;i++ ) {
            this->elements[i] = rhs.elements[i];
        }
    };

    // move:
    vector& operator=(const vector&& rhs) {
        this->alloc_object = move(rhs.alloc_object);
        this->elements = rhs.elements;
        this->array_size = rhs.array_size;
        this->object_count = rhs.object_count;
        rhs.elements = nullptr;
    };

    // destructor
    ~vector() {
        if( !(is_pointer<T>::value) ) {
            for(size_type i=0;i<this->object_count;i++) {
                alloc_object.destroy( this->elements+i );
            }
        }
        alloc_object.deallocate( this->elements, this->array_size );
    };

    allocator_type get_allocator() const { return this->alloc_object; };

    // member function "at":
    // TODO: add bounds checking
    reference at( size_type p ) { return this->elements[p]; };

    const_reference at( size_type p ) {
        return const_cast<const_reference>(this->elements[p]);
    } const;

    // access operator:
    reference operator[]( size_type p ) { return this->elements[p]; };

    const_reference operator[]( size_type p ) {
        return const_cast<const_reference>( this->elements[p] );
    };

    // member function "front":
    reference front() { return *this->elements; };

    const_reference front() {
        return const_cast<const_reference>(*(this->elements));
    };

    // member function "back":
    reference back() {
        if( this->object_count > 0 ) {
            return this->elements[this->object_count-1];
        } else {
            return this->elements[0];
        }
    };

    const_reference back() {
        if( this->object_count > 0 ) {
            return
            const_cast<const_reference>(this->elements[this->object_count-1]);
        } else {
            return const_cast<const_reference>(this->elements[0]);
        }
    };

    // member function "data":
    T* data() { return (T*)this->elements; };

    // iterator get functions:
    iterator end() {
        return this->elements+this->object_count;
    };

    const_iterator cend() {
        return const_cast<const_iterator>(this->elements+this->object_count);
    };

    // member function "clear":
    void clear() {
        if( !(is_pointer<T>::value) ) {
            for(size_type i=0;i<this->object_count;i++) {
                alloc_object.destruct( this->elements+i );
            }
        }
        this->object_count = 0;
    };

    // member function "pop back":
    void pop_back() {
        this->alloc_object.destruct( (this->elements+this->object_count)-1 );
        this->object_count--;
    };

    // member function "push_back"
    void push_back( const T& value ) {
        // object_count should never be > than array_size, but check anyways
        if( this->object_count >= this->array_size ) {
            // we need to resize before adding the element:
            this->reallocate( pow(this->array_size, 1.5) );
        }
        this->elements[this->object_count] = value;
        this->object_count++;
    };

    // member function "emplace_back":
    template< typename... Args >
    void emplace_back( Args&&... args ) {
        if( this->object_count >= this->array_size ) {
            this->reallocate( pow(this->array_size, 1.5) );
        }

        this->alloc_object.construct(
            this->elements+this->object_count, std::forward<Args>(args)... );
        this->object_count++;
    };

    // member function "insert":
    iterator insert( const_iterator pos, const T& value ) {
        difference_type insert_loc = (pos - this->elements);

        this->shift( pos, 1 );

        this->elements[insert_loc+1] = value;
    };

    iterator insert( const_iterator pos, const T&& value ) {
        difference_type insert_loc = (pos - this->elements);

        this->shift( pos, 1 );

        this->elements[insert_loc+1] = value;
    };

    // member function "emplace":
    template< typename... Args >
    iterator emplace( const_iterator pos, Args&&... args ) {
        difference_type insert_loc = (pos - this->elements);

        this->shift( pos, 1 );

        this->alloc_object.construct(
            const_cast<iterator>(pos+1), std::forward<Args>(args)... );

        return const_cast<iterator>(pos+1);
    };

    // member function "erase":
    iterator erase(const_iterator pos) {
        for(iterator i=const_cast<iterator>(pos);i!=(this->end()-1);i++) {
            this->alloc_object.destruct(i);
            *i = move(*(i+1));
        }
        this->object_count--;
        return const_cast<iterator>(pos+1);
    };

    iterator erase( const_iterator start, const_iterator end ) {
        iterator i2 = const_cast<iterator>(start);
        difference_type rem_size = (end - start);
        for(iterator i = const_cast<iterator>(end+1);i!=this->end();i++) {
            this->alloc_object.destruct(i2);
            *i2++ = move(*i);
        }
        this->object_count -= rem_size;
        return const_cast<iterator>(start+1);
    };

    // member function "assign":
    void assign( size_type count, const T& value ) {
        this->clear();
        for(size_type i=0;i<count;i++) {
            this->elements[i] = value;
        }
    };

    template< class InputIter >
    void assign( InputIter start, InputIter end ) {
        this->clear();
        while( start != end ) {
            this->push_back( *start++ );
        }
    };

    // member function "swap":
    void swap( vector& rhs ) {
        pointer tmp = rhs.elements;
        rhs.elements = this->elements;

        rhs::size_type t2 = rhs.object_count;
        rhs.object_count = this->object_count;
        this->object_count = t2;

        t2 = rhs.array_size;
        rhs.array_size = this->array_size;
        this->array_size = t2;

        rhs::allocator_type t3 = move(rhs.alloc_object);
        rhs.alloc_object = move(this->alloc_object);
        this->alloc_object = move(t3);
    };
};

};
