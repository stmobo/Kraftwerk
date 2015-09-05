// most of this is derived from the tutorial on osdev.
#include "interface/types.h"
#include "device/vga.h"

const unsigned int VGA_WIDTH = 80;
const unsigned int VGA_HEIGHT = 24;

unsigned int terminal_row;
unsigned int terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer;
 
char make_color(enum vga_color fg, enum vga_color bg)
{
	return fg | bg << 4;
}
 
uint16_t make_vgaentry(char c, uint8_t color)
{
	uint16_t c16 = c;
	uint16_t color16 = color;
	return c16 | color16 << 8;
}

 
void terminal_initialize()
{
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = make_color(COLOR_LIGHT_GREY, COLOR_BLACK);
	terminal_buffer = (uint16_t*) 0xB8000;
	for ( unsigned int y = 0; y < VGA_HEIGHT; y++ )
	{
		for ( unsigned int x = 0; x < VGA_WIDTH; x++ )
		{
			const unsigned int index = y * VGA_WIDTH + x;
			terminal_buffer[index] = make_vgaentry(' ', terminal_color);
		}
	}
}
 
void terminal_setcolor(char color)
{
	terminal_color = color;
}

// terminal_scroll - scroll the console
// Positive values scroll down (adding new lines to the bottom); negative values do the inverse.
void terminal_scroll(int num_rows)
{
    // lock here

    if(num_rows > 0) { // scroll down
        for(unsigned int y=0;y<VGA_HEIGHT-1;y++)
            for(unsigned int x=0;x<VGA_WIDTH;x++)
                terminal_buffer[y*VGA_WIDTH+x] = terminal_buffer[(y+1)*VGA_WIDTH+x];
        for(unsigned int x=0;x<VGA_WIDTH;x++)
            terminal_buffer[(VGA_HEIGHT-1)*VGA_WIDTH+x] = make_vgaentry(' ', make_color(COLOR_LIGHT_GREY, COLOR_BLACK));
    } else if(num_rows < 0) { // scroll up
        for(unsigned int y=VGA_HEIGHT-1;y>0;y--)
            for(unsigned int x=0;x<VGA_WIDTH;x++)
                terminal_buffer[y*VGA_WIDTH+x] = terminal_buffer[(y-1)*VGA_WIDTH+x];
        for(unsigned int x=0;x<VGA_WIDTH;x++)
            terminal_buffer[x] = make_vgaentry(' ', make_color(COLOR_LIGHT_GREY, COLOR_BLACK));
    }
}
 
// terminal_putentryat - write a character to screen, with color and position attributes.
// This function directly modifies the terminal buffer.
void terminal_putentryat(char c, char color, unsigned int x, unsigned int y)
{
    // don't lock
    
	const unsigned int index = y * VGA_WIDTH + x;
	terminal_buffer[index] = make_vgaentry(c, color);
}

// terminal_putchar - write a single character to screen
// This function prints a character to screen in a manner similar to "terminal_writestring" (see below).
// '\n' characters are automatically used to scroll and start new lines.
void terminal_putchar(char c)
{
    // lock here
    
    if(c=='\n') {
        terminal_column = 0;
        if ( ++terminal_row == VGA_HEIGHT ) {
            terminal_scroll(1);
            terminal_row = VGA_HEIGHT-1;
        }
        // unlock here
        return;
    }
    
	terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
    
	if ( ++terminal_column == VGA_WIDTH )
	{
		terminal_column = 0;
		if ( ++terminal_row == VGA_HEIGHT )
		{
			terminal_scroll(1);
            terminal_row = VGA_HEIGHT-1;
		}
	}
    
    // unlock here
}

void terminal_backspace() {
    // lock here
    if(--terminal_column > VGA_WIDTH) {
        if(--terminal_row > VGA_HEIGHT) {
            terminal_scroll(-1);
            terminal_row = 0;
        }
    }
    terminal_putentryat(' ', terminal_color, terminal_column, terminal_row);
    // unlock here
}
