#include "interface/print.h"
#include "interface/malloc.h"
#include "device/vga.h"

const char* alphanumeric = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
char *panic_str = NULL;
bool double_panic = false;

unsigned int digit_to_int( char digit ) {
    switch(digit) {
        case '1':
            return 1;
        case '2':
            return 2;
        case '3':
            return 3;
        case '4':
            return 4;
        case '5':
            return 5;
        case '6':
            return 6;
        case '7':
            return 7;
        case '8':
            return 8;
        case '9':
            return 9;
        case '0':
        default:
            return 0;
    }
}

int __copy_out( char* out, char* in, int max=0 ) {
    char cur = *in;
    int n_copied = 0;
    while( in[n_copied] != '\0' ) {
        out[n_copied] = in[n_copied];
        n_copied++;
        if( (max > 0) && (n_copied >= max) )
            break;
    }
#ifdef __PRINTF_HOSTED_TESTING_TRACE
    std::cout << "copied " << n_copied << " chars: " << in << std::endl;
#endif
    return n_copied;
}

size_t strlen( char* str ) {
    int i = 0;
    while( (*str++) != '\0' ) {
        i++;
    }
    return i;
}

size_t strlen( unsigned char* str ) {
    int i = 0;
    while( (*str++) != '\0' ) {
        i++;
    }
    return i;
}

char *concatentate_strings(char* str1, char *str2) {
    if( (str1 == NULL) && (str2 == NULL) )
        return NULL;
    if( str1 == NULL ) {
        char *out = (char*)kmalloc( strlen(str2)+1 );
        for(size_t i=0;i<strlen(str2);i++) {
            out[i] = str2[i];
        }
        return out;
    }
    if( str2 == NULL ) {
        char *out = (char*)kmalloc( strlen(str1)+1 );
        for(size_t i=0;i<strlen(str1);i++) {
            out[i] = str1[i];
        }
        return out;
    }
    int len = (strlen(str1) + strlen(str2));
    char *out = (char*)kmalloc(len + 1);
    for(size_t i=0;i<strlen(str1);i++) {
        out[i] = str1[i];
    }
    for(size_t i=0;i<strlen(str2);i++) {
        out[ i+strlen(str1) ] = str2[i];
    } 
    out[len] = '\0';
    return out;
}

char* __kvsprintf_justify( char* str, int width, bool left_justify, bool zero_pad ) {
    if( strlen(str) >= width ) {
        return str; // no need to justify
    }
    char padding_char = ' ';
    if( zero_pad )
        padding_char = '0';
    
    char* final = (char*)kmalloc(width+1);
    int n_padding_needed = width - strlen(str);
    if(left_justify) { // left justification - add padding to the end
        __copy_out(final, str);
        char* padding_ptr = final+strlen(str);
        for(int i=0;i<n_padding_needed;i++) {
            padding_ptr[i] = padding_char;
        }
    } else { // right justification - move every character n_padding_needed characters forward
        char* to_ptr = final+n_padding_needed;
        __copy_out(to_ptr, str);
        for(int i=0;i<n_padding_needed;i++) {
            final[i] = padding_char;
        }
    }
    final[width] = '\0';
    kfree( str );
    return final;
}

char* __kvsprintf_intprecision( char* str, int precision ) {
    int len = strlen(str);
    if( precision < len )
        return str;
    
    char *end = (char*)kmalloc(precision+1);
    int n_leading_zeroes = precision - len;
    for(int i=0;i<n_leading_zeroes;i++) {
        end[i] = '0';
    }
    __copy_out((end+n_leading_zeroes), str);
    kfree( str );
    return end;
}

/*
int get_n_digits( int num, int base )
{
	int i = 0;
	while(num > 0) {
		num /= base;
		i++;
	}
	return i;
}
*/

void reverse( char* str ) {
    for(int i=0, j=strlen(str)-1; i < j; i++, j--) {
        char c = str[i];
        str[i] = str[j];
        str[j] = c;
    }
}

signed int atoi( char* num ) {
    signed int n = 0;
    int i = 0;
    int len = strlen(num);
    bool neg = false;
    if( num[i] == '-' ) {
        neg = true;
        i++;
    }
    for(;i<len;i++) {
        n *= 10;
        n += digit_to_int( num[i] );
    }
    if( neg )
        n = -n;
    return n;
}

char* dtoa( double num, int precision, int base, bool add_space, bool add_plus ) {
#ifdef __PRINTF_HOSTED_TESTING_TRACE
    std::cout << "entering dtoa: " << num << ", precision=" << precision << std::endl;
#endif
    // quick and dirty (and probably inaccurate)
    bool neg = (num < 0);
    // convert integer portion first:
    if(neg)
        num = -num;
    uint64_t flr = (uint64_t)num;
    double frac = num - (uint64_t)num;
    int additional = 3;
    if( add_space || add_plus || neg )
        additional++;
    
    char* final = (char*)kmalloc( get_n_digits( flr, base ) + precision + additional );
    int i = 0;
    if( precision > 0 ) {
        char* frac_str = (char*)kmalloc(precision+1);
        
        uint64_t multiplier = base;
        while( (frac * multiplier) > int (frac * multiplier) ) {
            uint64_t shifted = (uint64_t)(frac * multiplier);
            multiplier *= base;
            
            frac_str[i++] = alphanumeric[shifted % base];
            if( i == precision ) {
                break;
            }
        }
        for(int j=0;j<i;j++) {
            final[j] = frac_str[(i-1) - j];
        }
        final[i++] = '.';
    }
    
    while( flr > 0 ) {
        final[i++] = alphanumeric[flr % base];
        flr /= base;
    }
    
    if( neg ) {
        final[i++] = '-';
    } else if(add_space) {
        final[i++] = ' ';
    } else if(add_plus) {
        final[i++] = '+';
    }
    final[i] = '\0';
    reverse(final);
#ifdef __PRINTF_HOSTED_TESTING_TRACE
    std::cout << "exiting dtoa: " << final << std::endl;
#endif
    return final;
}

char* dtosn( double num, int precision, int base, bool add_space, bool add_plus ) {
#ifdef __PRINTF_HOSTED_TESTING_TRACE
    std::cout << "entering dtosn" << std::endl;
#endif
    bool neg = (num < 0);
    bool neg_exponent = false;
    int additional = 3;
    if( add_space || add_plus || neg )
        additional++;
    if( neg )
        num = -num;
    
    // step 1: normalize (and get an exponent)
    int exponent = 0; // sign not looked at
    int multiplier = base;
    if( num < 1 ) { // shifting to the left
        neg_exponent = true;
        while( (num * multiplier) < 1 ) {
            multiplier *= base;
            exponent++;
        }
    } else { // shifting to the right
        while( (num / multiplier) > base ) {
            multiplier *= base;
            exponent++;
        }
    }
    
    if( neg_exponent )
        additional++;
    
    char *float_str = dtoa( num, precision, base, add_space, add_plus );
    char *exp_str   = itoa( exponent, base, false, false, false );
    int float_len   = strlen(float_str);
    char *final     = (char*)kmalloc( float_len + get_n_digits(exponent) + additional );
    int offset = 0;
    __copy_out(final, float_str);
    final[float_len] = 'e';
    final[float_len + 1] = '+';
    
    if( neg_exponent ) {
        final[float_len + get_n_digits(exponent) + 2] = '-';
        offset++;
    }
    final[float_len  + offset + get_n_digits(exponent) + 2] = '\0';
    __copy_out(final +(offset + float_len+2), exp_str );
    kfree(float_str);
    kfree(exp_str);
    return final;
}

/*
uint64_t fractional_to_binary( double frac ) {
    frac -= floor(frac);
    int n_bits = 0;
    uint64_t fractional = 0;
    while( (frac > 0) && (n_bits < 52) ) {
        frac *= 2;
        int tmp = floor(frac);
        if( tmp == 1 ) {
            fractional |= tmp;
            frac -= 1;
        }
        fractional <<= 1;
    }
    return fractional;
}
*/
// the above function converts a number between 0 and 1 into a series of fraction additions:
// (b1 / 2^-1) + (b2 / 2^-2) + (b3 / 2^-3) + ....

// F-to-A conversion:
// 1: denormalize the floating point number (lshift <mantissa> back by <exponent> bits)
// 2: separate integral / fractional parts
// 3: convert both separately back to integers? (somehow?) -- i lost my train of thought here

/*
char *ftoa_2( double num, char *str ) {
    uint64_t raw = *(reinterpret_cast<uint64_t*>(&num));
    uint64_t mantissa = raw & 0xFFFFFFFFFFFFF; // first 52 bits
    unsigned int raw_exponent = ((raw & (0x7FF << 52)) >> 52);
    bool sign = ((raw & (1<<63))>>63) & 1;
    signed int exponent = raw_exponent - 1023;
    mantissa = (1<<52) | mantissa;
    
}
*/

int kvsnprintf( char* out, size_t bufsz, const char* fmt, va_list args ) {
    unsigned int n_printed = 0;
    const char* pos = fmt;
    char  cur;
    bool discard = false;
    
    if( out == NULL )
        discard = true;
    
    while( (cur = *pos) != '\0' ) {
        if( cur == '%' ) {
            // FLAGS:
            bool left_justify = false; // '-'
            bool precede_sign = false; // '+'
            bool insert_space = false; // ' '
            bool insert_numer = false; // '#'
            bool zero_padding = false; // '0'
            
            char specifier = ' ';
            
            unsigned int precision = 0;
            bool precision_specified = false;
            
            unsigned int width = 0;
            bool width_specified = false;
            
            // LENGTH:
            bool hh = false;
            bool h  = false;
            bool l  = false;
            bool ll = false;
            bool j  = false;
            bool z  = false;
            bool t  = false;
            bool L  = false;
            
            int print_add = 0;
            
#ifdef __PRINTF_HOSTED_TESTING_TRACE
            std::cout << "Found %." << std::endl;
#endif
            
            pos++;
            if( (cur = *pos) == '%' ) {
                *out = cur;
                out++;
                goto end_specifier;
            }
            
            // Read flags:
            while( (cur = *pos) ) {
                switch(cur) {
                    case '-':
                        left_justify = true;
                        break;
                    case '+':
                        precede_sign = true;
                        break;
                    case ' ':
                        insert_space = true;
                        break;
                    case '#':
                        insert_numer = true;
                        break;
                    case '0':
                        zero_padding = true;
                        break;
                    default:
                        goto read_width;
                }
                pos++;
            }
            
            read_width:
#ifdef __PRINTF_HOSTED_TESTING_TRACE
            std::cout << "Reading width." << std::endl;
#endif
            while( (cur = *pos) ) {
                switch(cur) {
                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                        width *= 10;
                        width += digit_to_int(cur);
                        pos++;
                        width_specified = true;
                        break;
                    case '*':
                        width = va_arg(args, unsigned int);
                        width_specified = true;
                        pos++;
                    default:
                        goto read_precision;
                }
            }
            
            read_precision:
#ifdef __PRINTF_HOSTED_TESTING_TRACE
            std::cout << "Reading precision." << std::endl;
#endif
            if( (cur = *pos) == '.' ) {
                pos++;
                while( (cur = *pos) ) {
                    switch(cur) {
                        case '0':
                        case '1':
                        case '2':
                        case '3':
                        case '4':
                        case '5':
                        case '6':
                        case '7':
                        case '8':
                        case '9':
                            precision *= 10;
                            precision += digit_to_int(cur);
                            precision_specified = true;
                            break;
                        case '*':
                            precision = va_arg(args, unsigned int);
                            precision_specified = true;
                        default:
                            goto read_length;
                    }
                    pos++;
                }
            }
            
            read_length:
            if( !precision_specified ) {
                precision = 6;
            }
#ifdef __PRINTF_HOSTED_TESTING_TRACE
            std::cout << "Reading length." << std::endl;
#endif
            switch( (cur = *pos) ) {
                case 'h':
                    if( (*++pos) == 'h' ) {
                        hh = true;
                        pos++;
                    } else {
                        h = true;
                    }
                    break;
                case 'l':
                    if( (*++pos) == 'l' ) {
                        ll = true;
                        pos++;
                    } else {
                        l = true;
                    }
                    break;
                case 'j':
                    j = true;
                    pos++;
                    break;
                case 'z':
                    z = true;
                    pos++;
                    break;
                case 't':
                    t = true;
                    pos++;
                    break;
                case 'L':
                    L = true;
                    pos++;
                    break;
            }
            
            read_specifier:
#ifdef __PRINTF_HOSTED_TESTING_TRACE
            std::cout << "Reading specifier." << std::endl;
#endif
            specifier = *pos;
            
            if( specifier == 'n' ) {
                if( hh ) {
                    signed char* ptr = va_arg(args, signed char*);
                    *ptr = n_printed;
                } else if(h) {
                    short int* ptr = va_arg(args, short int*);
                    *ptr = n_printed;
                } else if(l) {
                    long int* ptr = va_arg(args, long int*);
                    *ptr = n_printed;
                } else if(ll) {
                    long long int* ptr = va_arg(args, long long int*);
                    *ptr = n_printed;
                } else if(j) {
                    intmax_t* ptr = va_arg(args, intmax_t*);
                    *ptr = n_printed;
                } else if(t) {
                    size_t* ptr = va_arg(args, size_t*);
                    *ptr = n_printed;
                } else if(z) {
                    ptrdiff_t* ptr = va_arg(args, ptrdiff_t*);
                    *ptr = n_printed;
                } else {
                    int* ptr = va_arg(args, int*);
                    *ptr = n_printed;
                }
                goto end_specifier;
            }
            
            // now construct the string.
            
            // first, get a char array long enough to hold any possible value, plus prefixes
            // for 'd' and 'i', this is log10( pow(2, (8*sizeof(input value type))) ) + 1 (for any possible '-' signs) + 1 (for the null terminator)
            // for 'u', this is the above minus 1 (since there's no possible '-' sign)
            
            // the following sizes apply:
            // 8 bits  - (unsigned) char
            // 16 bits - (unsigned) short int
            // 32 bits - (unsigned) (long) int, size_t, ptrdiff_t, pointers
            // 64 bits - (unsigned) long long int, (u)intmax_t
            
#ifdef __PRINTF_HOSTED_TESTING_TRACE
            std::cout << "Constructing string. (specifier = " << specifier << ")" << std::endl;
#endif
            switch(specifier) {
                case 'd':
                case 'i':
                {
                    char *final;
                    if( hh ) {
                        int val1 = va_arg(args, int);
                        signed char val2 = (signed char)val1;
                        
                        final = itoa(val2, 10, insert_space, precede_sign, insert_numer);
                    } else if( h ) {
                        int val1 = va_arg(args, int);
                        short int val2 = (short int)val1;
                        
                        final = itoa(val2, 10, insert_space, precede_sign, insert_numer);
                    } else if( l ) {
                        long int val = va_arg(args, long int);
                        
                        final = itoa(val, 10, insert_space, precede_sign, insert_numer);
                    } else if( ll ){
                        long long int val = va_arg(args, long long int);
                        
                        final = itoa(val, 10, insert_space, precede_sign, insert_numer);
                    } else if( j ) {
                        intmax_t val = va_arg(args, intmax_t);
                        
                        final = itoa(val, 10, insert_space, precede_sign, insert_numer);
                    } else if( z ) {
                        size_t val = va_arg(args, size_t);
                        
                        final = itoa(val, 10, insert_space, precede_sign, insert_numer);
                    } else if( t ) {
                        ptrdiff_t val = va_arg(args, ptrdiff_t);
                        
                        final = itoa(val, 10, insert_space, precede_sign, insert_numer);
                    } else {
                        int val = va_arg(args, int);
                        
                        final = itoa(val, 10, insert_space, precede_sign, insert_numer);
                    }
                    
                    if( precision_specified )
                        final = __kvsprintf_intprecision( final, precision );
                    if( width_specified )
                        final = __kvsprintf_justify( final, width, left_justify, zero_padding );
#ifdef __PRINTF_HOSTED_TESTING_TRACE
                    std::cout << " __copy_out at line " << __LINE__ << std::endl;
#endif
                    if( !discard ) {
                        print_add += __copy_out(out, final);
                    } else {
                        print_add += strlen(final);
                    }
                    kfree(final);
                    break;
                } // end case 'd' / 'i'
                case 'u':
                case 'o':
                case 'x':
                case 'X':
                {
                    char *final;
                    if( hh ) {
                        unsigned int val1 = va_arg(args, unsigned int);
                        unsigned char val = (unsigned char)val1;
                        
                        if( specifier == 'u' ) {
                            final = itoa(val, 10, insert_space, precede_sign, insert_numer);
                        } else if( specifier == 'o' ) {
                            final = itoa(val, 8, insert_space, precede_sign, insert_numer);
                        } else {
                            final = itoa(val, 16, insert_space, precede_sign, insert_numer);
                        }
                    } else if( h ) {
                        unsigned int val1 = va_arg(args, unsigned int);
                        unsigned short int val = (unsigned short int)val1;
                        
                        if( specifier == 'u' ) {
                            final = itoa(val, 10, insert_space, precede_sign, insert_numer);
                        } else if( specifier == 'o' ) {
                            final = itoa(val, 8, insert_space, precede_sign, insert_numer);
                        } else {
                            final = itoa(val, 16, insert_space, precede_sign, insert_numer);
                        }
                    } else if( l ) {
                        unsigned long int val = va_arg(args, unsigned long int);
                        
                        if( specifier == 'u' ) {
                            final = itoa(val, 10, insert_space, precede_sign, insert_numer);
                        } else if( specifier == 'o' ) {
                            final = itoa(val, 8, insert_space, precede_sign, insert_numer);
                        } else {
                            final = itoa(val, 16, insert_space, precede_sign, insert_numer);
                        }
                    } else if( ll ){
                        unsigned long long int val = va_arg(args, unsigned long long int);
                        
                        if( specifier == 'u' ) {
                            final = itoa(val, 10, insert_space, precede_sign, insert_numer);
                        } else if( specifier == 'o' ) {
                            final = itoa(val, 8, insert_space, precede_sign, insert_numer);
                        } else {
                            final = itoa(val, 16, insert_space, precede_sign, insert_numer);
                        }
                    } else if( j ) {
                        uintmax_t val = va_arg(args, uintmax_t);
                        
                        if( specifier == 'u' ) {
                            final = itoa(val, 10, insert_space, precede_sign, insert_numer);
                        } else if( specifier == 'o' ) {
                            final = itoa(val, 8, insert_space, precede_sign, insert_numer);
                        } else {
                            final = itoa(val, 16, insert_space, precede_sign, insert_numer);
                        }
                    } else if( z ) {
                        size_t val = va_arg(args, size_t);
                        
                        if( specifier == 'u' ) {
                            final = itoa(val, 10, insert_space, precede_sign, insert_numer);
                        } else if( specifier == 'o' ) {
                            final = itoa(val, 8, insert_space, precede_sign, insert_numer);
                        } else {
                            final = itoa(val, 16, insert_space, precede_sign, insert_numer);
                        }
                    } else if( t ) {
                        ptrdiff_t val = va_arg(args, ptrdiff_t);
                        
                        if( specifier == 'u' ) {
                            final = itoa(val, 10, insert_space, precede_sign, insert_numer);
                        } else if( specifier == 'o' ) {
                            final = itoa(val, 8, insert_space, precede_sign, insert_numer);
                        } else {
                            final = itoa(val, 16, insert_space, precede_sign, insert_numer);
                        }
                    } else {
                        unsigned int val = va_arg(args, unsigned int);
                        
                        if( specifier == 'u' ) {
                            final = itoa(val, 10, insert_space, precede_sign, insert_numer);
                        } else if( specifier == 'o' ) {
                            final = itoa(val, 8, insert_space, precede_sign, insert_numer);
                        } else {
                            final = itoa(val, 16, insert_space, precede_sign, insert_numer);
                        }
                    }
                    
                    if( precision_specified )
                        final = __kvsprintf_intprecision( final, precision );
                    if( width_specified )
                        final = __kvsprintf_justify( final, width, left_justify, zero_padding );
#ifdef __PRINTF_HOSTED_TESTING_TRACE
                    std::cout << " __copy_out at line " << __LINE__ << std::endl;
#endif
                    if( !discard ) {
                        print_add += __copy_out(out, final);
                    } else {
                        print_add += strlen(final);
                    }
                    kfree(final);
                    break;
                } // end case 'u' / 'o' / 'x' / 'X'
                /*
		case 'f':
                case 'F':
                {   
                    char *final;
                    if( L ) {
                        long double val = va_arg(args, long double);
                        final = dtoa( val, precision, 10, insert_space, precede_sign );
                    } else {
                        double val = va_arg(args, double);
                        final = dtoa( val, precision, 10, insert_space, precede_sign );
                    }
                    if( width_specified )
                        final = __kvsprintf_justify( final, width, left_justify, zero_padding );
#ifdef __PRINTF_HOSTED_TESTING_TRACE
                    std::cout << " __copy_out at line " << __LINE__ << std::endl;
#endif
                    if( !discard ) {
                        print_add += __copy_out(out, final);
                    } else {
                        print_add += strlen(final);
                    }
                    kfree(final);
                    break;
                } // end case 'f' / 'F'
		
                case 'e':
                case 'E':
                {
                    if( !precision_specified ) {
                        precision = 6;
                    }
                    char *final;
                    if( L ) {
                        long double val = va_arg(args, long double);
                        final = dtosn( val, precision, 10, insert_space, precede_sign );
                    } else {
                        double val = va_arg(args, double);
                        final = dtosn( val, precision, 10, insert_space, precede_sign );
                    }
                    if( width_specified )
                        final = __kvsprintf_justify( final, width, left_justify, zero_padding );
#ifdef __PRINTF_HOSTED_TESTING_TRACE
                    std::cout << " __copy_out at line " << __LINE__ << std::endl;
#endif
                    if( !discard ) {
                        print_add += __copy_out(out, final);
                    } else {
                        print_add += strlen(final);
                    }
                    kfree(final);
                    break;
                } // end case 'e' / 'E'
		
                case 'a':
                case 'A':
                {
                    if( !precision_specified ) {
                        precision = 6;
                    }
                    char *final;
                    if( L ) {
                        long double val = va_arg(args, long double);
                        final = dtosn( val, precision, 16, insert_space, precede_sign );
                    } else {
                        double val = va_arg(args, double);
                        final = dtosn( val, precision, 16, insert_space, precede_sign );
                    }
                    if( width_specified )
                        final = __kvsprintf_justify( final, width, left_justify, zero_padding );
#ifdef __PRINTF_HOSTED_TESTING_TRACE
                    std::cout << " __copy_out at line " << __LINE__ << std::endl;
#endif
                    if( !discard ) {
                        print_add += __copy_out(out, final);
                    } else {
                        print_add += strlen(final);
                    }
                    kfree(final);
                    break;
                } // end case 'a' / 'A'
                case 'g':
                case 'G':
                {
                    if( !precision_specified ) {
                        precision = 6;
                    }
                    char *final1;
                    char *final2;
                    char *final;
                    if( L ) {
                        long double val = va_arg(args, long double);
                        final1 = dtoa( val, precision, 10, insert_space, precede_sign );
                        final2 = dtosn( val, precision, 10, insert_space, precede_sign );
                        
                        if(strlen(final1) > strlen(final2)) {
                            final = final2;
                        } else {
                            final = final1;
                        }
                    } else {
                        double val = va_arg(args, double);
                        final1 = dtoa( val, precision, 10, insert_space, precede_sign );
                        final2 = dtosn( val, precision, 10, insert_space, precede_sign );
                        
                        if(strlen(final1) > strlen(final2)) {
                            final = final2;
                            kfree(final1);
                        } else {
                            final = final1;
                            kfree(final2);
                        }
                    }
                    
                    if( width_specified )
                        final = __kvsprintf_justify( final, width, left_justify, zero_padding );
#ifdef __PRINTF_HOSTED_TESTING_TRACE
                    std::cout << " __copy_out at line " << __LINE__ << std::endl;
#endif
                    if( !discard ) {
                        print_add += __copy_out(out, final);
                    } else {
                        print_add += strlen(final);
                    }
                    kfree(final);
                    break;
                } // end case 'g' / 'G'
		*/
                case 'c':
                {
                    if(l) {
                        unsigned int val1 = va_arg(args, unsigned int);
                        // can't copy a wchar_t to an array of chars-- just drop it silently
                    } else {
                        int val1 = va_arg(args, int);
                        char val = (char)val1;
                        
                        if( !discard )
                            *out = val;
                        print_add++;
                    }
                    break;
                } // end case 'c'
                case 's':
                {
                    if( !l ) {
                        char *ptr = va_arg(args, char*);
                        char *final;
                        if( precision_specified ) {
                            final = (char*)kmalloc( precision+1 );
                            __copy_out(final, ptr, precision);
                        } else {
                            final = (char*)kmalloc( strlen(ptr)+1 );
                            __copy_out(final, ptr);
                        }
                        if( width_specified )
                            final = __kvsprintf_justify( final, width, left_justify, zero_padding );
#ifdef __PRINTF_HOSTED_TESTING_TRACE
                        std::cout << " __copy_out at line " << __LINE__ << std::endl;
#endif
                        if( !discard ) {
                            print_add += __copy_out(out, final);
                        } else {
                            print_add += strlen(final);
                        }
                        kfree(final);
                    } else {
                        wchar_t *final = va_arg(args, wchar_t*);
                        // see above, with 'c'
                    }
                    break;
                } // end case 's'
                case 'p':
                {
                    void* ptr = va_arg(args, void*);
                    uintptr_t val = reinterpret_cast<uintptr_t>(ptr);
                    
                    char *final = itoa(val, 16, false, false, insert_numer);
                    //char *final = concatentate_strings("p", initial);
                    if( width_specified )
                        final = __kvsprintf_justify( final, width, left_justify, zero_padding );
#ifdef __PRINTF_HOSTED_TESTING_TRACE
                    std::cout << " __copy_out at line " << __LINE__ << std::endl;
#endif
                    if( !discard ) {
                        print_add += __copy_out(out, final);
                    } else {
                        print_add += strlen(final);
                    }
                    kfree(final);
                    break;
                } // end case 'p'
                // ignore any unknown specifiers
            }
            
            end_specifier:
            pos++;
            n_printed += print_add;
            if( !discard ) {
                out += print_add;
            }
        } else {
            if( !discard ) {
                *out = cur;
                out++;
            }
            pos++;
            n_printed++;
#ifdef __PRINTF_HOSTED_TESTING_TRACE
            std::cout << "dumping character to output:" << cur << std::endl;
#endif
        }
        if( (bufsz > 0) && (n_printed >= (bufsz-1)) ) {
            discard = true;
        }
    }
    if( out != NULL ) {
        *out = '\0';
        out++;
    }
    
    return n_printed;
}

int ksprintf(char *out, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int ret = kvsnprintf(out, 0, fmt, args);
    va_end(args);
    return ret;
}

int kvsprintf(char *out, const char *fmt, va_list args) {
    int ret = kvsnprintf(out, 0, fmt, args);
    return ret;
}

int ksnprintf(char *out, size_t bufsz, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int ret = kvsnprintf(out, bufsz, fmt, args);
    va_end(args);
    return ret;
}


int kvprintf(const char *fmt, va_list args) {
	va_list args2;

	va_copy(args2, args);

	int n = kvsnprintf(NULL, 0, fmt, args2);
	char *t = new char[n+1];
	kvsnprintf(t, n+1, fmt, args);
	terminal_writestring( t );
	delete t;

	va_end(args2);

	return n;
}

// Print something to screen.
int kprintf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    
    int ret = kvprintf(fmt, args);
    
    va_end(args);
    
    return ret;
}

// Print "panic: mesg" and then hang.
void panic(char *str, ...) {
    if(double_panic) { // triple panic / fault!
        panic_str = str;
        while(true) {
            asm volatile("cli\n\t"
            "hlt"
            : : : "memory");
        }
    }
    if(panic_str != NULL) { // Don't panic WHILE panicking.
        double_panic = true;
        terminal_writestring("panic: panicked while panicking!\n");
        terminal_writestring("first panic string: ");
        terminal_writestring(panic_str);
        terminal_writestring("\nsecond panic string: ");
        terminal_writestring(str);
    } else {
        panic_str = str; // save the unformatted string just in case
        
        va_list args;
        va_list args2;
        va_start(args, str);
        va_copy(args2, args);
        
        int len = kvsprintf(NULL, str, args);
        char* buf = (char*)kmalloc(len+1);
        
        kvsnprintf(buf, len+1, str, args2);
        panic_str = buf; // okay, save the formatted string since we now have it.
        
        terminal_writestring("panic: ");
        terminal_writestring(buf);
        
        va_end(args);
        va_end(args2);
    }
    // Now halt.
    while(true) {
        asm volatile("cli\n\t"
        "hlt"
        : : : "memory");
    }
}
