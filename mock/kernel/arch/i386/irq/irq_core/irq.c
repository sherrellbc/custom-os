#include <mock.h>
#include <arch/descriptor.h>
#include <arch/irq.h>
#include <arch/pic8259.h>
#include <arch/apic.h>


irq_handler_t irq_get_handler(irq_t slot)
{
    struct gate_desc desc;

    idt_get_slot(slot, &desc);
    return (irq_handler_t) ((desc.handler_addr1 << sizeof(irq_handler_t)/2) & (desc.handler_addr0));
}


kern_return_t irq_insert_handler(irq_handler_t handler, irq_t slot)
{
    struct gate_desc desc = LDT_DESCRIPTOR_ENTRY(handler, SELECTOR(0,0,1), 0x8e);

    idt_set_slot(slot, &desc); 
    printk("Install idt[%d] = 0x%x\n", slot, handler);

    return KERN_SUCCESS;
}


kern_return_t irq_remove_handler(irq_handler_t *handler, irq_t slot)
{
    *handler = irq_get_handler(slot);
    idt_set_slot(slot, NULL);
    printk("Remove idt[%d] = 0x%x\n", slot, NULL);

    return KERN_SUCCESS;
}


static void arch_global_irq_enable(void)
{
    asm("sti"); 
}


static void arch_global_irq_disable(void)
{
    asm("cli");
}


static int irq_hw_detect(void)
{
    //TODO: Do we have pic, apic, xapic, x2apic, etc
    return IRQ_HW_PIC8259;
}


//TODO: Detect hardware, then delegate this function assignment to the file
void irq_init(void)
{
    /* Set the functions that are not IRQ hardware-specific */
    plat.irq_global_disable = arch_global_irq_disable;
    plat.irq_global_enable  = arch_global_irq_enable;
    plat.irq_hdlr_ins       = irq_insert_handler;
    plat.irq_hdlr_rem       = irq_remove_handler;
    plat.irq_hdlr_get       = irq_get_handler;
       
    /* Detect what hardware we have and set the utility functions */
    switch(irq_hw_detect()){
        case IRQ_HW_APIC:
        case IRQ_HW_PIC8259:
            plat.irq_init   = pic8259_init;
            plat.irq_disable= pic8259_mask_irq;
            plat.irq_enable = pic8259_unmask_irq;
            plat.irq_set    = pic8259_irq_set;
            plat.irq_get    = pic8259_irq_get;
            break;

        default:
            break;
    }  

    plat.irq_init();
    plat.irq_global_disable();

    //FIXME: Remove this once implemented properly
    plat.irq_hdlr_ins(time_systick_handler, 32);
    plat.irq_hdlr_ins(kb_handler_entry, 33);
    plat.irq_hdlr_ins(int_entry_cmos, 40);
//    plat.irq_insert(int_entry_spurious, 39);
    plat.irq_hdlr_ins(int_entry_spurious, 47);
    cmos_init();
}
