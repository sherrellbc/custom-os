#ifndef _PLATFORM_H
#define _PLATFORM_H

#include <irq.h>


struct platform {
    /* Interrupts */
    void (*irq_init)(void);  
    void (*irq_global_disable)(void);  
    void (*irq_global_enable)(void);  
    void (*irq_disable)(irq_t irq);  
    void (*irq_enable)(irq_t irq);  
    void (*irq_setmask)(irq_t mask);  
};


/* For use all about the kernel */
extern struct platform plat;


#endif /* _PLATFORM_H */
