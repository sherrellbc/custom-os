#ifndef _SATIRE_KERNEL_H
#define _SATIRE_KERNEL_H

#include "kdebug.h"


/*
 * Main entry point for the kernel proper. This entry function is called after 
 * all the platform-specific initialization code is executed
 */
void kernel_main(void);


/*
 * Exit panic. This serves as a safe-catch for when logic errors force a function
 * to return when the design dictated it should never happen
 * 
 * This function never returns
 */
void exit_panic(void);


//FIXME
void kpanic(const char *str, int num);

#endif /* _SATIRE_KERNEL_H */
