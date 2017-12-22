#ifndef _INTERRUPT_H
#define _INTERRUPT_H

#include <stdint.h>

void irq_disable(unsigned int);
void irq_enable(unsigned int);
void irq_setmask(uint16_t mask);


#endif /* _INTERRUPT_H */
