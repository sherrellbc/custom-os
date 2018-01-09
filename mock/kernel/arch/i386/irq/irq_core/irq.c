#include <platform.h>
#include <arch/descriptor.h>
#include <arch/irq.h>
#include <arch/pic8259.h>
#include <arch/apic.h>


void arch_global_irq_enable(void)
{
    asm("sti"); 
}


void arch_global_irq_disable(void)
{
    asm("cli");
}


static int irq_hw_detect(void)
{
    return IRQ_HW_PIC8259;
}


extern void kb_handler_entry(void);
extern void time_systick_handler(void);
extern uint32_t *g_default_handlers;
void irq_init(void)
{
    plat.irq_global_disable =   arch_global_irq_disable;
    plat.irq_global_enable =    arch_global_irq_enable;

    switch(irq_hw_detect()){
        case IRQ_HW_APIC:
        case IRQ_HW_PIC8259:
            plat.irq_init =     pic8259_init;
            plat.irq_disable =  pic8259_mask_irq;
            plat.irq_enable =   pic8259_unmask_irq;
            plat.irq_setmask =  pic8259_setmask;
            break;

        default:
            break;
    }  

    plat.irq_init();
    plat.irq_global_disable();
    plat.irq_setmask(0xffff);

    //FIXME: Remove this from this file
//    struct gate_desc kb = LDT_DESCRIPTOR_ENTRY((uint32_t) kb_handler_entry, SELECTOR(0,0,1), 0x8e);
//    struct gate_desc time = LDT_DESCRIPTOR_ENTRY((uint32_t) time_systick_handler, SELECTOR(0,0,1), 0x8e);
//    idt_set_slot(8, &time); 
//    idt_set_slot(9, &kb); 
}
