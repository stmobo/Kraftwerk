namespace standalone_std {

template<typename T>
class weak_ptr {
    counter* ctr_object;
    T* ptr;
    
    void increment() { if(this->ctr_object) { this->ctr_object->weak_count++; } };
    void decrement() { if(this->ctr_object) { this->ctr_object->weak_count--; } };
public:
    typedef T element_type;
    
    // constructors:
    
    // default:
    explicit weak_ptr() : ctr_object(nullptr), ptr(nullptr) {};
    
    // copy:
    explicit weak_ptr( const weak_ptr& rhs ) {
        this->ctr_object = rhs.ctr_object;
        this->ptr = rhs.ptr;
        
        increment();
    };
    
    template<typename U>
    explicit weak_ptr( const weak_ptr<U>& rhs ) {
        this->ctr_object = rhs.ctr_object;
        this->ptr = rhs.ptr;
        
        increment();
    };
    
    template<typename U>
    explicit weak_ptr( const shared_ptr<U>& rhs ) {
        this->ctr_object = rhs.ctr_object;
        this->ptr = rhs.ptr;
        
        increment();
    };
    
    // move:
    explicit weak_ptr( const weak_ptr&& rhs ) {
        this->ctr_object = rhs.ctr_object;
        this->ptr = rhs.ptr;
        
        rhs.ctr_object = nullptr;
        rhs.ptr = nullptr;
    };
    
    template<typename U>
    explicit weak_ptr( const weak_ptr<U>&& rhs ) {
        this->ctr_object = rhs.ctr_object;
        this->ptr = rhs.ptr;
        
        rhs.ctr_object = nullptr;
        rhs.ptr = nullptr;
    };
    
    // destructor:
    ~weak_ptr() {
        decrement();
    };
    
    // assignment operators:
    // copy:
    weak_ptr& operator=( const weak_ptr& rhs ) {
        this->ctr_object = rhs.ctr_object;
        this->ptr = rhs.ptr;
        increment();
        
        return *this;
    };
    
    template<typename U>
    weak_ptr& operator=( const weak_ptr<U>& rhs ) {
        this->ctr_object = rhs.ctr_object;
        this->ptr = rhs.ptr;
        increment();
        
        return *this;
    };
    
    template<typename U>
    weak_ptr& operator=( const shared_ptr<U>& rhs ) {
        this->ctr_object = rhs.ctr_object;
        this->ptr = rhs.ptr;
        increment();
        
        return *this;
    };
    
    // move:
    weak_ptr& operator=( const weak_ptr&& rhs ) {
        this->ctr_object = rhs.ctr_object;
        this->ptr = rhs.ptr;
        
        rhs.ctr_object = nullptr;
        rhs.ptr = nullptr;
        
        return *this;
    };
    
    template<typename U>
    weak_ptr& operator=( const shared_ptr<U>&& rhs ) {
        this->ctr_object = rhs.ctr_object;
        this->ptr = rhs.ptr;
        
        rhs.ctr_object = nullptr;
        rhs.ptr = nullptr;
        
        return *this;
    };
    
    // member functions:
    void reset() {
        decrement();
        
        rhs.ctr_object = nullptr;
        rhs.ptr = nullptr;
    };
    
    void swap( weak_ptr& rhs ) {
        element_type* tmp = this->ptr;
        counter* tmp2 = this->ctr_object;
        
        this->ctr_object = rhs.ctr_object;
        this->ptr = rhs.ptr;
        
        rhs.ctr_object = tmp2;
        rhs.ptr = tmp;
    };
    
    unsigned int use_count() {
        if( this->ctr_object ) {
            return this->ctr_object->strong_count;
        } else {
            return 0;
        }
    };
    
    bool expired() {
        if (this->ctr_object) {
            return (this->ctr_object->strong_count == 0);
        }
        return false;
    };
    
    shared_ptr<T> lock() {
        if (this->ctr_object) {
            return shared_ptr<T>(*this);
        } else {
            return shared_ptr<T>();
        }
    };
    
    
};

};