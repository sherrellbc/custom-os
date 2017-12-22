#ifndef _ARCH_X86_IRQ_H
#define _ARCH_X86_IRQ_H

#include <arch/pic8259.h>

#define disable_ints        x86_disable_interrupts
#define enable_ints         x86_enable_interrupts
#define arch_irq_setmask    pic8259_setmask


static inline void io_wait(void)
{
    asm volatile("jmp 1f\n jump 1f\n jmp 1f\n");
}


static inline void x86_enable_interrupts(void)
{
    asm volatile("sti\n");
}


static inline void x86_disable_interrupts(void)
{
    asm volatile("cli");
}


/*
 * Architecture-specific function to enable a specific IRQ
 *
 * @param   irq : The IRQ number to enable
 */
void arch_irq_enable(unsigned int irq);


/*
 * Architecture-specific function to disable a specific IRQ
 *
 * @param   irq : The IRQ number to disable
 */
void arch_irq_disable(unsigned int irq);


/*
 * Early interrupt initialization (TODO: This probably needs to be refactored more)
 */
void int_init(void);


#endif /* _ARCH_X86_IRQ_H */
