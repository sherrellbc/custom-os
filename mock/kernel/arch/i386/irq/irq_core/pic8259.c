#include <arch/irq.h>
#include <arch/pic8259.h>


/* Local data structure used to keep track of the current PIC configuration */
struct pic8259_conf {
    uint8_t master_voffset; 
    uint8_t slave_voffset;
} g_pic8259_conf = {0};


static void pic8259_remap(uint8_t moffset, uint8_t soffset)
{
    return;
    pic_outb(PIC8259_MASTER_CMD, 0x11);
    pic_outb(PIC8259_SLAVE_CMD, 0x11);

    pic_outb(PIC8259_MASTER_DATA, moffset);
    pic_outb(PIC8259_SLAVE_DATA, soffset);

    pic_outb(PIC8259_MASTER_DATA, 4);
    pic_outb(PIC8259_SLAVE_DATA, 2);

    pic_outb(PIC8259_MASTER_DATA, 0x01);
    pic_outb(PIC8259_SLAVE_DATA, 0x01);

    g_pic8259_conf.master_voffset = moffset;
    g_pic8259_conf.slave_voffset = soffset;
}


void pic8259_init(void)
{
    uint16_t mask = pic8259_getmask();
    pic8259_remap(0x20, 0x28);
    pic8259_setmask(mask);
}


void pic8259_disable(void)
{
    pic8259_setmask(0xffff);
}


uint16_t pic8259_get_register(int reg)
{
    pic_outb(PIC8259_MASTER_CMD, reg);
    pic_outb(PIC8259_SLAVE_CMD, reg);
    return (pic_inb(PIC8259_SLAVE_CMD) << 8) | pic_inb(PIC8259_MASTER_CMD);
}


uint16_t pic8259_getmask(void)
{
    return (pic_inb(PIC8259_SLAVE_DATA) << 8) | pic_inb(PIC8259_MASTER_DATA);
}


void pic8259_mask_irq(irq_t irq)
{
    pic8259_setmask(pic8259_getmask() | (1 << irq));
}


void pic8259_unmask_irq(irq_t irq)
{
    pic8259_setmask(pic8259_getmask() & ~(1 << irq));    
    pic8259_flush();    //XXX: Is this necessary?
}


void pic8259_setmask(irq_t mask)
{
    pic_outb(mask & 0xff, PIC8259_MASTER_DATA);
    pic_outb(mask >> 8, PIC8259_SLAVE_DATA);
}


void pic8259_flush(void)
{    
    pic_outb(PIC8259_CMD_FLUSH_ISR, PIC8259_MASTER_CMD);
    pic_outb(PIC8259_CMD_FLUSH_ISR, PIC8259_SLAVE_CMD);
}


void pic8259_eoi(void)
{
    //FIXME: send to correct pic
    pic_outb(PIC8259_MASTER_CMD, 0x20);
    pic_outb(PIC8259_SLAVE_CMD, 0x20);
}

