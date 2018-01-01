#include <stdarg.h>
#include <stdio.h>

 
int printk(const char *format, ...) {
    static char buf[1024]; //TODO: remove this static buffer requirement
    int ret;

    va_list arg;
    va_start(arg, format);
    vsnprintf(buf, sizeof(buf), format, arg);
    va_end(arg);

    ret = 0;
    while(putchar(buf[ret++]) != '\0')
        ;

    return ret;
}
