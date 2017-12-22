#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include <kernel/tty.h>
#include <arch/vga.h>

 
#define VGA_MEM ((uint16_t*) 0xB8000)
 

/* Structure representing the current state of the VGA hardware config */
//FIXME: This should really be a driver
struct vga_info {
    int width;
    int height;
    uint16_t *buffer;
    int row;
    int col;
    int color;
}g_vga = {  
            .width = 80,
            .height = 25,
            .buffer = VGA_MEM,
            .row = 0,
            .col = 0,
            .color = 0
         };


//TODO: modeset to higher resolution
void early_console_init(void)
{
	g_vga.color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    console_clear();
}


void console_clear(void)
{
    int row,col;
	for (row = 0; row < g_vga.height; row++){
		for (col = 0; col < g_vga.width; col++){
			g_vga.buffer[row*g_vga.width + col] = vga_entry(' ', g_vga.color);
		}
	}
}


void console_set_color(uint8_t color)
{
    g_vga.color = color;
}


void console_putentryat(char c, uint8_t color, size_t col, size_t row)
{
	const size_t index = row * g_vga.width + col;
	g_vga.buffer[index] = vga_entry(c, color);
}


char console_putc(char c)
{
    /* Special case characters */
    switch(c){
    case '\n':
        if( ++g_vga.row == g_vga.height )
            g_vga.row = 0;
        __attribute__((fallthrough));
    case '\r':
        g_vga.col = 0;
        __attribute__((fallthrough));
    case '\0':
        return c;

    default:
        break;
    }

	console_putentryat(c, g_vga.color, g_vga.col, g_vga.row);
	if (++g_vga.col == g_vga.width) {
		g_vga.col = 0;
		if (++g_vga.row == g_vga.height)
			g_vga.row = 0;
	}
    
    return c;
}


size_t console_write(const uint8_t *data, size_t len)
{
	for (size_t i = 0; i < len; i++)
		console_putc(data[i]);

    return len;
}


size_t console_puts(const char *str)
{
	return console_write((const uint8_t *)str, strlen(str));
}
