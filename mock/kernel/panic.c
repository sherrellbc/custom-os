#include <kernel/printk.h>


void exit_panic(void)
{
    printk("Kernel PANIC! We should not have exited ...");
    
    volatile int fixme=1;
    while(fixme)
        ;
}


//FIXME: variadic function
void kpanic(char *str, int num)
{
    int actual = 0;
    for(num >>=1; num != 0; num >>=1, actual++);
    printk(str, actual);
    
    volatile int fixme=1;
    while(fixme)
        ;
}
