#include <mock.h>


void irq_enable(irq_t irq)
{
    plat.irq_enable(irq);
}


void irq_disable(irq_t irq)
{
    plat.irq_disable(irq);
}


//TODO: TBD on how to do this with APIC ...
void irq_setmask(irq_t mask)
{
    (void) mask;
//    plat.irq_setmask(mask);
}
