#ifndef _ARCH_X86_IRQ_H
#define _ARCH_X86_IRQ_H

#include "irq/time.h"
#include "irq/cmos.h"
#include "irq/keyboard.h"


/* The default system handler, used to detect programming/configuration issues */
void default_handler(void);


enum irq_hw {
    IRQ_HW_PIC8259,
    IRQ_HW_APIC,
};

/*
 * Early interrupt initialization 
 */
void irq_init(void);


/*
 * Platform global interrupt enable
 */
//void arch_global_irq_enable(void);


/*
 * Platform global interrupt disable
 */
//void arch_global_irq_disable(void);


#endif /* _ARCH_X86_IRQ_H */
