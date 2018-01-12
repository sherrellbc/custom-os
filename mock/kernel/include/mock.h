#ifndef _MOCK_H
#define _MOCK_H


/* Simple, portable method of determining which bit-ness we are compiling for */
#if UINTPTR_MAX     == 0xffffffff
    #define KERN_32
#elif UINTPTR_MAX   == 0xffffffffffffffff
    #define KERN_64
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <kern_return.h>
#include <platform.h>
#include <kernel/printk.h>
#include <kernel/tty.h>


/*
 * Main entry point for the kernel proper. This entry function is called after 
 * all the platform-specific initialization code is executed. It begins by
 * initializing all platform subsystems and, ultimately, starting the first
 * userspace process
 */
void kernel_main(void);


/*
 * Exit panic. This serves as a safe-catch for when logic errors force a function
 * to return when the design dictated it should never happen
 * 
 * This function never returns
 */
//void exit_panic(void);


//XXX: This should be in a "debug" area of the headers, and defined only if a debug build
#define assertk(expr) ({                                                    \
            if(0 == (expr)){                                                \
                printk("Assert failed: %s:%d\n", __FUNCTION__, __LINE__);   \
                while(1);                                                   \
            }                                                               \
        })                                                                  \


//void kpanic(const char *str, int num);

#endif /* _MOCK_H */
