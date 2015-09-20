#pragma once
#include "interface/types.h"
#include "interface/malloc.h"
#include <stdarg.h>

extern const char* alphanumeric;

char* concatentate_strings(char*, char*);
void  reverse(char*);
signed int atoi( char* );

template<typename int_type>
int get_n_digits( int_type num, int base=10 )
{
	int i = 0;
	while(num > 0) {
		num /= base;
		i++;
	}
	return i;
}

template<typename int_type>
char* itoa( int_type num, int base, bool add_space, bool add_plus, bool add_prefix )
{
	bool neg = ((num < 0) && (base == 10));
	int len = 0;
	int additional = 1;

	if( add_space || add_plus || neg ) {
		additional++;
	}
	
	if( add_prefix && (base == 8) )
		additional++;
	
	if( add_prefix && (base == 16) )
		additional += 2;
	
	char *str = (char*)kmalloc( get_n_digits(num, base)+additional );
	
	if(neg)
		num = -num;

	do {
		str[len++] = alphanumeric[num%base];
	} while( (num /= base) > 0 );
	
	if( add_prefix ) {
		if( base == 8 ) {
			str[len++] = '0';
		} else if(base == 16) {
			str[len++] = 'x';
			str[len++] = '0';
		}
	}
	
	if(neg) {
		str[len++] = '-';
	} else if (add_space) {
		str[len++] = ' ';
	} else if (add_plus) {
		str[len++] = '+';
	}
	
	str[len] = '\0';

	// now reverse it
	reverse(str);

	return str;
}

char* dtoa( double num, int precision=6, int base=10, bool add_space=false, bool add_plus=false );
char* dtosn( double num, int precision=18, int base=10, bool add_space=false, bool add_plus=false );
int   kvsnprintf(char* out, size_t bufsz, const char* fmt, va_list args);
int   kvsprintf(char* out, const char* fmt, va_list args);
int   kvprintf(const char* fmt, va_list args);
int   ksnprintf(char* out, size_t bufsz, const char* fmt, ...);
int   ksprintf(char* out, const char* fmt, ...);
int   kprintf(const char* fmt, ...);
void  panic(char*, ...);

#define      __kassert_stringifier_2(x) #x
#define      __kassert_stringifier_1(x) __kassert_stringifier_2(x)
#define      kassert( exp, message ) do { if( !(exp) ) { panic( __FILE__ " ( " __kassert_stringifier_1(__LINE__) " ): " message ); } } while(0)