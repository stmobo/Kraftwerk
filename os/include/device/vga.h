#pragma once
#include "interface/types.h"

extern const unsigned int VGA_WIDTH;    // 80
extern const unsigned int VGA_HEIGHT;   // 24

extern unsigned int terminal_row;
extern unsigned int terminal_column;

extern uint8_t terminal_color;
extern uint16_t* terminal_buffer;
enum vga_color
{
	COLOR_BLACK = 0,
	COLOR_BLUE = 1,
	COLOR_GREEN = 2,
	COLOR_CYAN = 3,
	COLOR_RED = 4,
	COLOR_MAGENTA = 5,
	COLOR_BROWN = 6,
	COLOR_LIGHT_GREY = 7,
	COLOR_DARK_GREY = 8,
	COLOR_LIGHT_BLUE = 9,
	COLOR_LIGHT_GREEN = 10,
	COLOR_LIGHT_CYAN = 11,
	COLOR_LIGHT_RED = 12,
	COLOR_LIGHT_MAGENTA = 13,
	COLOR_LIGHT_BROWN = 14,
	COLOR_WHITE = 15,
};

extern char make_color(enum vga_color, enum vga_color);
extern void terminal_initialize();
extern void terminal_setcolor(char);
extern void terminal_scroll(int);
extern void terminal_putentryat(char, char, unsigned int, unsigned int);
extern void terminal_putchar(char);

// terminal_writestring - print a string to screen
// this function prints a line of text to screen, wrapping and scrolling if necessary.
template<typename string_type>
void terminal_writestring(string_type data)
{
    // lock here
	//unsigned int datalen = strlen(data);
	//for ( unsigned int i = 0; i < datalen; i++ ) {
    unsigned int i = 0;
    while( data[i] != 0 ) {
		terminal_putchar(data[i]);
        i++;
    }
    // unlock here
}

template<typename string_type>
void terminal_writestring(string_type data, unsigned int datalen)
{
	for ( unsigned int i = 0; i < datalen; i++ )
		terminal_putchar(data[i]);
}

extern const char* alphanumeric;
template<typename int_type>
void terminal_writehex(int_type data)
{
	unsigned int n_chars = (sizeof(data)*2);
	for(unsigned int i=0;i<n_chars;i++) {
		terminal_putchar(alphanumeric[(data>>((n_chars-i)-1)*4)&0xF]);
	}
}

extern void terminal_backspace();
