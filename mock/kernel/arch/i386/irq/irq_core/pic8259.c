#include <arch/irq.h>
#include <arch/pic8259.h>
#include <mock.h>

/*
 * Theory
 *
 * The 8259 programmable interrupt controller (PIC) was used in the early
 * 8086 (and later) days for managing external interrupts. The device supports
 * eight discrete priority-driven interrupts, while being connected to the CPU
 * proper with only one interrupt line. The 8259 also supports being used in
 * a cascade configuration in which an additional 8259 is chained together 
 * in such a way (typically connected to IR2 with two pics) that an additional 
 * eight interrupts is supported. In this configuration, the first 8259 is often 
 * referred to as the 'master' and the other the 'slave'. Also noteworthy, due to this 
 * chaining mode, the IRx line used in the cascade is not a _real_ interrupt in the 
 * common sense. It is rather a signal to the master pic informing that it should 
 * signal the CPU regarding an interrupt status unknown to it from the slave
 * device. In cascade mode, the 8259 will also automatically (per its configuration)
 * determine which slave device was the source of the interrupt, select it by asserting
 * its own cascade pins (CAS0-2) side the slave ID and, thus, allow it to directly 
 * send its 8-bit data word onto the system bus. In this mode, each slave device
 * also has its /INTA line directly connected to the core (as does the master), so
 * when the CPU is servicing the interrupt all cycles are passed directly to it; the
 * master pic knows to ignore these cycles. However, in this mode the CPU must
 * explicitly send an EOI to the slave that cause the interrupt _and_ the master
 * device (since its IRx line was raised by a slave).
 *
 * Occasionally, the interrupt request at the 8259 will disappear before the core
 * has a chance to service it. In this case, IRQ7 is asserted and passed to the core
 * as if it has been the original source of interrupts. This is known as the
 * 'Suprious Interrupt' case and must be handled in software.
 *
 * The process looks like this:
 *
 * 1) One or more IRQ lines on the 8259 are raised high, setting the corresponding
 *    bits in the Interrupt Request Register (IRR) register.
 *
 * 2) The 8259 evaluates the request and raises the CPU's INT line, if approporate.
 *    e.g. the signal is not sent if the interrupt is masked out in the Interrupt
 *    Mask Register (IMR), or if an interrupt currently being serviced by the CPU
 *    has higher priority than the one just triggered (see ISR register).
 *
 * 3) The CPU automatically (in hardware) acknowledges the INT signal by responding
 *    with a /INTA pulse.
 *
 * 4) Upon receiving the /INTA pulse, the 8259 will set the IRQ (the one just
 *    triggered) with the highest priority (recal many may come in at once) in 
 *    the Interrupt Service Register (ISR) and clear it from the IRR.
 *
 * 5) The core will initiate a second /INTA pulse, during which the 8259 releases
 *    an 8-bit value (the interrupt number) on the Data Bus. This data is read 
 *    by the CPU.
 *
 * 6) The 'interrupt cycle' is now complete. Depending on the PIC's configured
 *    mode, two things will now happen. In Automatic End of Interrupt (AEOI) mode, 
 *    the second /INTA pulse from step 5 will cause the recently set ISR bit to 
 *    reset. Otherwise, the set ISR bit remains in this state until the CPU issues 
 *    an End of Interrupt (EOI) command.
 *
 *
 * ----------------------------------------------------------------------------
 * Programming the 8259
 * 
 * Initialization Command Words (ICW)
 * - When a command is issued with A0=0 and D4=1, the contents are interpreted as
 *   ICW1. Once the initialization sequence is started, the following occurs:
 *
 *      1) Edge-sense is reset to trigger on low->high transitions
 *      2) IMR is cleared
 *      3) IR7 is given priority 7
 *      4) The slave mode address is set to 7
 *      5) Special Mask Mode is cleared and Status Read is set to IRR
 *      6) If IC4=0, then all functions selected in ICW4 are set to zero
 *
 * Operation Command Words (OCW)
 */


/*
 * Definitions for the ICW words relevent to the x86 implementation
 */
#define ICW1_IC4_NEEDED     (1 << 0)
#define ICW1_IC4_NOT_NEEDED (0 << 0)
#define ICW1_SINGLE_MODE    (1 << 1)
#define ICW1_CASCADE_MODE   (0 << 1)
#define ICW1_TRIG_LEVEL     (1 << 3)
#define ICW1_TRIG_EDGE      (0 << 3)
#define ICW1_D4_BEGIN_ICW   (1 << 4)

#define ICW4_uPM_8086       (1 << 0)
#define ICW4_AEOI           (1 << 1)
#define ICW4_MASTER_DEVICE  (1 << 2)
#define ICW4_SLAVE_DEVICE   (0 << 2)
#define ICW4_MODE_BUFFERED  (1 << 3)
#define ICW4_MODE_SPF       (1 << 4)

/*
 * Definitions for the OCW words relevent to the x86 implementation
 */
#define OCW2_NON_SPECIFIC_EOI   (1 << 5)
#define OCW3_READ_REGISTER      (1 << 1)
#define OCW3_IN_SERVICE_REG     (1 << 0)
#define OCW3_INT_REQUEST_REG    (0 << 0)
#define PIC8259_REG_IRR         (0x08 | OCW3_READ_REGISTER | OCW3_INT_REQUEST_REG)
#define PIC8259_REG_ISR         (0x08 | OCW3_READ_REGISTER | OCW3_IN_SERVICE_REG)

/*
 * There is no real process for deciding where to remap the pic. We just need
 * to ensure the remap does not overlap protected mode interrupts (e.g. 0-31). 
 * As such, we can define the default remapping here for simplicity
 */
#define PIC8259_MASTER_REMAP_BASE   (0x20)
#define PIC8259_SLAVE_REMAP_BASE    (PIC8259_MASTER_REMAP_BASE + 8)

/* 
 * Local data structure used to keep track of the current PIC configuration 
 */
struct pic8259_conf {
    uint8_t active;
    uint8_t master_voffset; 
    uint8_t slave_voffset;
} g_pic8259_conf = {0};


static void pic8259_remap(uint8_t moffset, uint8_t soffset)
{
    /* ICW1: Edge triggered (default), cascade mode (deafult), ICW4 is needed */
    pic_outb(PIC8259_MASTER_CMD, ICW1_D4_BEGIN_ICW | ICW1_IC4_NEEDED);
    pic_outb(PIC8259_SLAVE_CMD,  ICW1_D4_BEGIN_ICW | ICW1_IC4_NEEDED);

    /* ICW2: Fully nexted mode (default), vector offset (rebase) */
    pic_outb(PIC8259_MASTER_DATA, moffset);
    pic_outb(PIC8259_SLAVE_DATA, soffset);

    /* 
     * IWC3:
     *      For Master: Configure which pin is connected in cascade mode to a slave
     *      For Slave:  Set the slave ID (for systems with many slaves)
     *
     *  Note: We inform the master that a slave device is connected to its IRQ2 line,
     *  while informing the slave that its identity is also two
     */
    pic_outb(PIC8259_MASTER_DATA, (1 << 2));
    pic_outb(PIC8259_SLAVE_DATA,        2);

    /* ICW4: Non-buffered mode (default), EOI required (default), 8086 mode */ 
    pic_outb(PIC8259_MASTER_DATA, ICW4_uPM_8086);
    pic_outb(PIC8259_SLAVE_DATA,  ICW4_uPM_8086);

    /* Update our local configuration. Recall only b7-b3 are used when rebasing the 8259 */
    assertk( 0 == ((moffset & 0x7) | (soffset & 0x7)) );
    g_pic8259_conf.master_voffset = moffset & 0xf8;
    g_pic8259_conf.slave_voffset =  soffset & 0xf8;
}


static uint16_t pic8259_get_register(int reg)
{
    pic_outb(PIC8259_MASTER_CMD, 0x08 | OCW3_READ_REGISTER | reg);
    pic_outb(PIC8259_SLAVE_CMD,  0x08 | OCW3_READ_REGISTER | reg);
    return (pic_inb(PIC8259_SLAVE_CMD) << 8) | pic_inb(PIC8259_MASTER_CMD);
}


void pic8259_disable(void)
{
    pic8259_setmask(0xffff);
    g_pic8259_conf.active = 0;
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
}


void pic8259_setmask(irq_t mask)
{
    pic_outb(PIC8259_MASTER_DATA, mask & 0xff);
    pic_outb(PIC8259_SLAVE_DATA, mask >> 8);
}


uint16_t pic8259_get_irr(void)
{
    return pic8259_get_register(OCW3_INT_REQUEST_REG);
}


uint16_t pic8259_get_isr(void)
{
    return pic8259_get_register(OCW3_IN_SERVICE_REG);
}


void pic8259_eoi(void)
{
    /* Send the slave an EOI if it was the source of the interrupt */
    if(pic8259_get_isr() & 0xff00){
        pic_outb(PIC8259_SLAVE_CMD, OCW2_NON_SPECIFIC_EOI);
    }

    /*
     * Either the master was the source of the interrupt or it was
     * a proxy for the slave. Either way we need to send an EOI
     */
    pic_outb(PIC8259_MASTER_CMD, OCW2_NON_SPECIFIC_EOI);
}


int pic8259_is_active(void)
{
    return (1 == g_pic8259_conf.active);
}


void pic8259_init(void)
{
    pic8259_setmask(0xffff);
    pic8259_remap(PIC8259_MASTER_REMAP_BASE, PIC8259_SLAVE_REMAP_BASE);
    g_pic8259_conf.active = 1;
}

