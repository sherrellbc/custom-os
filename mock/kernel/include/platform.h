#ifndef _PLATFORM_H
#define _PLATFORM_H

#include <mock.h>
#include <irq.h>


struct platform {
    /* Interrupts */
    void (*irq_init)(void);
    void (*irq_global_disable)(void);
    void (*irq_global_enable)(void);
    void (*irq_disable)(irq_t irq);
    void (*irq_enable)(irq_t irq);
    int  (*irq_get)(irq_t irq);
    int (*irq_set)(irq_t irq, int state);

    kern_return_t (*irq_hdlr_ins)(irq_handler_t handler, irq_t slot);
    kern_return_t (*irq_hdlr_rem)(irq_handler_t *handler, irq_t slot);
    irq_handler_t (*irq_hdlr_get)(irq_t slot);

    /* Time */
};


/* For use all about the kernel */
extern struct platform plat;


#endif /* _PLATFORM_H */
