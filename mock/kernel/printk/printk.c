#include <stdarg.h>
#include <stdio.h>

 
int printk(const char* restrict fmt, ...) {
    static char buf[1024]; //TODO: remove this static buffer requirement
    int ret;

    va_list arg;
    va_start(arg, fmt);
    vsnprintf(fmt, sizeof(buf), buf, arg);
    va_end(arg);

    ret = 0;
    while(putchar(buf[ret++]) != '\0')
        ;

    return ret;
}
