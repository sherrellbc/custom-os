#include <platform.h>
#include <kernel/kernel.h>
#include <kernel/printk.h>
#include <kernel/tty.h>

#include <multiboot/multiboot.h>
#include <arch/descriptor.h>
#include <arch/irq.h>


/* Instance of the global platform structure */
struct platform plat = {0}; 


void x86_boot_legacy(struct multiboot_info* mbi, uint32_t magic)
{
    early_console_init();

    if(0 != mb_init(mbi, magic)){
        printk("The Multiboot header failed validation!\n");
        abort();
    }

    /* Install the default GDT and IDT */
    gdt_setup();
    idt_setup();
    
    /* Initialize the interrupt subsystem; returns with interrupts disabled */
    irq_init();

    /* Jump to the kernel proper's main entry point */
    kernel_main();
}
