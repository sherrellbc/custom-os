#ifndef _KERNEL_TTY_H
#define _KERNEL_TTY_H

#include <stddef.h>
#include <stdint.h>


/* 
 * Initialize the early console to support printing boot messages
 */
void early_console_init(void);


/*
 * Simply clear the current contents of the console
 */
void console_clear(void);


/* 
 * Write a character to the console
 * @param   char : Character to write
 */
char console_putc(char c);


/*
 * Write raw bytes to the console
 * @param   data : Data buffer to write
 * @param   len  : Length of buffer
 * @return       : Number of bytes written
 */
size_t console_write(const uint8_t* data, size_t len);


/*
 * Write a (nul-terminated!) string to the console
 * @param   str : String to write
 * @return      : Number of bytes written
 */
size_t console_puts(const char* str);


/*
 * Set the color of for the next character output to the console
 * @param   color : The desired color, per the VGA spec
 */
//void console_set_color(uint8_t color);

 
#endif

