namespace standalone_std {

template< typename T, Del = default_deleter<T> >
class unique_ptr {
    T* stored_pointer;
    Del deleter;
    
public:
    typedef remove_reference<T>::type element_type;
    typedef remove_reference<deleter_type>::type deleter_type;
    typedef remove_reference<T>::type* pointer;
    
    // constructors:
    // default:
    explicit unique_ptr() : stored_pointer(nullptr), deleter(deleter_type()) {};
    explicit unique_ptr(nullptr_t) : stored_pointer(nullptr), deleter(deleter_type()) {};
    // with pointer:
    explicit unique_ptr(element_type* ptr) : stored_pointer(ptr), deleter(deleter_type()) {};
    // with pointer and copied deleter:
    explicit unique_ptr(element_type* ptr, remove_reference<deleter_type>::type& del) : stored_pointer(ptr), deleter(del) {};
    // with pointer and moved deleter:
    explicit unique_ptr(element_type* ptr, remove_reference<deleter_type>::type&& del) : stored_pointer(ptr), deleter(del) {};
    // move constructors:
    explicit unique_ptr( const unique_ptr&& rhs ) {
        this->stored_pointer = rhs.stored_pointer;
        this->deleter = move(rhs.deleter);
        
        rhs.stored_pointer = nullptr;
    };
    
    template<typename U, D>
    explicit unique_ptr( const unique_ptr<U, D>&& rhs ) {
        this->stored_pointer = rhs.stored_pointer;
        this->deleter = move(rhs.deleter);
        
        rhs.stored_pointer = nullptr;
    };
    
    // copy constructor (nonexistent):
    unique_ptr(const unique_ptr&) = delete;
    
    // destructor:
    ~unique_ptr() {
        if( this->stored_pointer != nullptr ) {
            this->deleter(this->stored_pointer);
        }
    };
    
    // assignment operators:
    // move assignment
    unique_ptr& operator=(const unique_ptr&& rhs) {
        if( this->stored_pointer != nullptr ) {
            this->deleter(this->stored_pointer);
        }
        
        this->stored_pointer = rhs.stored_pointer;
        this->deleter = move(rhs.deleter);
        
        rhs.stored_pointer = nullptr;
        
        return *this;
    };
    // nullptr assignment
    unique_ptr& operator=(const nullptr_t rhs) {
        if( this->stored_pointer != nullptr ) {
            this->deleter(this->stored_pointer);
        }
        
        this->stored_pointer = nullptr_t;
        this->deleter = deleter_type();
        
        return *this;
    };
    // typecast move assignment
    template<typename U, D>
    unique_ptr& operator=(const unique_ptr<U,D>&& rhs) {
        if( this->stored_pointer != nullptr ) {
            this->deleter(this->stored_pointer);
        }
        
        this->stored_pointer = rhs.stored_pointer;
        this->deleter = move(rhs.deleter);
        
        rhs.deleter = D();
        rhs.stored_pointer = nullptr;
        
        return *this;
    };
    // copy assignment (deleted)
    unique_ptr& operator=(const unique_ptr& rhs) = delete;
    
    // member functions:
    element_type* get() {
        return this->stored_pointer;
    };
    
    deleter_type& get_deleter() {
        return this->deleter;
    };
    
    bool operator bool() {
        return (this->stored_pointer != nullptr);
    };
    
    pointer release() {
        pointer tmp = this->stored_pointer;
        this->stored_pointer = nullptr;
        return tmp;
    };
    
    void reset( pointer p = pointer() ) {
        if( this->stored_pointer == nullptr ) {
            this->deleter(this->stored_pointer);
        }
        this->stored_pointer = p;
    };
    
    void swap( unique_ptr& rhs ) {
        pointer tmp = this->stored_pointer;
        this->stored_pointer = rhs.stored_pointer;
        rhs.stored_pointer = tmp;
        
        deleter_type tmp2 = move(this->deleter);
        this->deleter = move(rhs.deleter);
        rhs.deleter = move(tmp2);
    };
    
    element_type& operator*() {
        return *this->stored_pointer;
    };
    
    pointer operator->() {
        return this->stored_pointer;
    };
};

};