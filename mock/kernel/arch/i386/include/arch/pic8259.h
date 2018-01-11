#ifndef _PIC8259_H
#define _PIC8259_H

#include <stdint.h>
#include <irq.h>
#include <arch/io.h>
#include <mock.h>


/* Location of Command and data ports for the Master/Slave PIC */
#define PIC8259_MASTER_CMD      0x20
#define PIC8259_MASTER_DATA     0x21

#define PIC8259_SLAVE_CMD       0xa0
#define PIC8259_SLAVE_DATA      0xa1


/* 
 * Apparently reading and writing to the PIC without regulation can introduce 
 * nasty latency bugs. So, we'll just create this inline to make sure we do not
 * run into such a problem 
 */
static inline uint8_t pic_inb(unsigned int port)
{   
    uint8_t value = inb(port);
    //need io delay to PIC
    return value;
}


static inline void pic_outb(uint8_t port, unsigned int value)
{
    outb(port, value);
    //need io delay to PIC
}


void pic8259_init(void);
void pic8259_mask_irq(irq_t irq);
void pic8259_unmask_irq(irq_t irq);
void pic8259_setmask(irq_t mask);
uint16_t pic8259_getmask(void);
void pic8259_flush(void);
void pic8259_eoi(void);

uint16_t pic8259_get_irr(void);
uint16_t pic8259_get_isr(void);


#endif /* _PIC8259_H */
