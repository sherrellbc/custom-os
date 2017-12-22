#include <kernel/kdebug.h>
#include <arch/irq.h>
#include <arch/pic8259.h>

#include <kernel/printk.h>

uint16_t pic8259_get_register(int reg)
{
    pic_outb(PIC8259_MASTER_CMD, reg);
    pic_outb(PIC8259_SLAVE_CMD, reg);
    return (pic_inb(PIC8259_SLAVE_DATA) << 8) | pic_inb(PIC8259_MASTER_DATA);
}


uint16_t pic8259_getmask(void)
{
    return (pic_inb(PIC8259_SLAVE_DATA) << 8) | pic_inb(PIC8259_MASTER_DATA);
}


void pic8259_mask_irq(unsigned int irq)
{
    pic8259_setmask(pic8259_getmask() | (1 << irq));
}


void pic8259_unmask_irq(unsigned int irq)
{
    pic8259_setmask(pic8259_getmask() & ~(1 << irq));    
}


void pic8259_setmask(uint16_t mask)
{
    pic_outb(mask & 0xff, PIC8259_MASTER_DATA);
    pic_outb(mask >> 8, PIC8259_SLAVE_DATA);
}


void pic8259_flush(void)
{    
    pic_outb(PIC8259_CMD_FLUSH_ISR, PIC8259_MASTER_CMD);
    pic_outb(PIC8259_CMD_FLUSH_ISR, PIC8259_SLAVE_CMD);
}
