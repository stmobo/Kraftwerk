namespace standalone_std {

    template<typename T> struct remove_reference      { typedef T type; };
    template<typename T> struct remove_reference<T&>  { typedef T type; };
    template<typename T> struct remove_reference<T&&> { typedef T type; };
    
    template<typename T> struct remove_pointer                      {typedef T type; };
    template<typename T> struct remove_pointer<T*>                  {typedef T type; };
    template<typename T> struct remove_pointer<T* const>            {typedef T type; };
    template<typename T> struct remove_pointer<T* volatile>         {typedef T type; };
    template<typename T> struct remove_pointer<T* const volatile>   {typedef T type; };
    
    template<typename T> struct add_pointer {typedef typename remove_reference<T>::type* type};

    template<typename T> struct remove_const<T> { typedef T type; };
    template<typename T> struct remove_const<const T> { typedef T type; };
    template<typename T> struct remove_volatile<T> { typedef T type; };
    template<typename T> struct remove_volatile<volatile T> { typedef T type; };
    template<typename T> struct remove_cv { typedef typename remove_volatile<remove_const<T>::type>::type type };
    
    template<typename T>
    typename remove_reference<T>::type&&
    forward(typename remove_reference<T>::type& t) {
        return static_cast<T&&>(t);
    }
    
    template<typename T>
    typename remove_reference<T>::type&&
    forward(typename remove_reference<T>::type&& t) {
        return static_cast<T&&>(t);
    }
    
    template<typename T>
    typename remove_reference<T>::type&&
    move(T&& t) {
        return t;
    }
    
    template<typename T>
    void swap(T& a, T& b) {
        T tmp(move(a));
        a = move(b);
        b = move(tmp);
    }

};