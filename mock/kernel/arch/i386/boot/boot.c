#include <kernel/kernel.h>
#include <kernel/printk.h>
#include <kernel/tty.h>

#include <stdlib.h>
#include <multiboot/multiboot.h>
#include <arch/descriptor.h>
#include <arch/io.h>
#include <arch/irq.h>


//FIXME: Figure out a decent method for computing delays
void udelay(unsigned long usecs){
    for(; usecs != 0; usecs--){
        if(inb(0x80) || 1)
            continue;
    }
}


void x86_boot(struct multiboot_info* mbi, uint32_t magic)
{
    early_console_init();

    if(0 != mb_init(mbi, magic)){
        printk("The Multiboot header failed validation!\n");
        abort();
    }
    
    /* Configure PIC or APIC; disable interrupts */
    int_init();

    /* Install early default GDT and IDT */
    gdt_setup();
    idt_setup();

    kernel_main();
}
