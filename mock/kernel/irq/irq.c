#include <mock.h>


void irq_enable(irq_t irq)
{
    plat.irq_enable(irq);
}


void irq_disable(irq_t irq)
{
    plat.irq_disable(irq);
}


void irq_setmask(irq_t mask)
{
    plat.irq_setmask(mask);
}
