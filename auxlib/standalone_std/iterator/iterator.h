template< class Category,
  typename T,
  typename Distance=ptrdiff_t,
  typename Pointer = T*,
  typename Reference = T&
  >
struct iterator {
    typedef Category iterator_category;
    typedef T value_type;
    typedef Distance difference_type;
    typedef Pointer pointer;
    typedef Reference reference;
};
