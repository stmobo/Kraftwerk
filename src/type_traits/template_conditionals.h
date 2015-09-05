namespace standalone_std {

template< typename T, T Value >
struct integral_constant {
    typedef T value_type;
    typedef integral_constant<T, Value> type;

    static constexpr T value = Value;

    constexpr operator value_type() const noexcept { return Value; };
    constexpr value_type operator ()() const { return Value; };
};

typedef integral_constant<bool, true> true_type;
typedef integral_constant<bool, false> false_type;

template< typename T, typename U >
struct is_same : false_type {};

template< typename T >
struct is_same<T, T> : true_type {};

template< typename T > struct is_ptr_helper : std::false_type {};
template< typename T > struct is_ptr_helper<T*> : std::true_type {};
template< typename T > struct is_pointer :
    is_pointer_helper< remove_cv<T>::type > {};

template< bool C, class T, class F >
struct conditional { typedef T type; };

template< class T, class F >
struct conditional<false, T, F> { typedef F type; };

#define generate_has_typedef(Name)                                   \
template<class T>                                                    \
class has_typename_Name {                                        \
typedef char yes[1];                                                 \
typedef char no[2];                                                  \
                                                                     \
template<class C>                                                    \
static yes& check(typename C::Name * member);                      \
                                                                     \
template<typename>                                                   \
static no& check(...);                                               \
                                                                     \
public:                                                              \
static const bool value = (sizeof(check<T>(nullptr)) == sizeof(yes));\
};



};
