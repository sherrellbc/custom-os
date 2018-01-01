#ifndef _STDIO_H
#define _STDIO_H

#include <stddef.h>
#include <stdarg.h>

 
//int printf(const char* __restrict, ...);
char putchar(char);
//int puts(const char*);
int snprintf(char *buffer, size_t size, const char *fmt, ...);
int vsnprintf(char *buf, size_t len, const char *format, va_list va);


#endif /* _STDIO_H */
