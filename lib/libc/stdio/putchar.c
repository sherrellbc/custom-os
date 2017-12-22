#include <stdio.h>

#ifdef __KERN__
#include <kernel/tty.h>
#endif


char putchar(char c) {
#ifdef __KERN__
	return console_putc(c);
#endif
    //TODO: userspace implementation
    return c;
}
