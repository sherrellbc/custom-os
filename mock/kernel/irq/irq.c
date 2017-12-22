#include <kernel/interrupt.h>
#include <arch/irq.h>



void irq_enable(unsigned int irq)
{
    arch_irq_enable(irq);
}


void irq_disable(unsigned int irq)
{
    arch_irq_disable(irq);
}


void irq_setmask(uint16_t mask)
{
    //Do other interrupt bookkeeping (to keep us from having to do this slow IO)
    arch_irq_setmask(mask);
}
