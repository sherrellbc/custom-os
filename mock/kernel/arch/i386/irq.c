#include <arch/irq.h>
#include <stdint.h>


void int_init(void)
{
    pic8259_setmask(0xffff);
    x86_disable_interrupts();
}


void arch_irq_enable(unsigned int irq)
{
    (void)irq;
    /* Mask out all interrupts */
//    pic8259_setmask(0xffff & ~(1<<irq));
//    pic8259_setmask(0x0011);
    pic8259_unmask_irq(irq); 

    /* Clear pending interrupts */
    pic8259_flush();
    x86_enable_interrupts(); //FIXME: Why is this here?
}


void arch_irq_disable(unsigned int irq)
{
    pic8259_mask_irq(irq);
    x86_disable_interrupts();
}


void arch_disable_all_ints(void)
{
    pic8259_setmask(0xffff);
}
