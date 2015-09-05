namespace standalone_std {

struct counter {
    unsigned int strong_count;
    unsigned int weak_count;
    
    counter() : strong_count(1), weak_count(0) {};
    virtual void destroy()=0;
    virtual ~counter() {};
};

template<typename U, Del>
struct counter_actual : public counter {
    U* real_ptr;
    Del deleter;
    
    counter_actual( U* ptr, Del x ) : counter(), real_ptr(ptr), deleter(x) {};
    virtual void destroy() { deleter(real_ptr); };
};

};