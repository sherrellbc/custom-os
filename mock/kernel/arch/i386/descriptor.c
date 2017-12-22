#include <kernel/kernel.h>
#include <kernel/printk.h>
#include <arch/descriptor.h>

#include <arch/io.h>

uint64_t dt_create_descriptor(uint32_t base, uint32_t limit, uint16_t flags)
{
    uint64_t desc = 0;

    desc  = ((base & 0xffff) << 16) | (limit & 0xffff);
    desc |= (base & 0xff000000) | ((flags & 0xd0ff) << 8) | (limit & 0xf0000) | (base & 0xff0000 >> 16);

    return desc;
}


void reload_segments(uint16_t seg_code, uint16_t seg_data)
{
    (void) seg_code;
    (void) seg_data;

    asm volatile(   "jmp $0x08,$load_cs   \n\t"
                    "load_cs:           \n\t"
                    "mov $0x10, %%ax    \n\t"
                    "mov %%ax, %%ds     \n\t"
                    "mov %%ax, %%es     \n\t"
                    "mov %%ax, %%fs     \n\t"
                    "mov %%ax, %%gs     \n\t"
                    "mov %%ax, %%ss     \n\t"
                    ::: );
}


void gdt_setup(void)
{
    static const struct segment_desc early_gdt[] __attribute__((aligned(16))) = {
        [0] = GDT_DESCRIPTOR_ENTRY(0,0,0),                      /* First required empty/null descriptor for error detection */
        [1] = GDT_DESCRIPTOR_ENTRY(0, 0xfffff, (GDT_CODE_PL0)), /* Next two entries are for kernel space; DPL=0 */
        [2] = GDT_DESCRIPTOR_ENTRY(0, 0xfffff, (GDT_DATA_PL0)),
        [3] = GDT_DESCRIPTOR_ENTRY(0, 0xfffff, (GDT_CODE_PL3)), /* Below two entries are for userspace; DPL=3*/
        [4] = GDT_DESCRIPTOR_ENTRY(0, 0xfffff, (GDT_DATA_PL3))
    };

    static struct desc_table_ptr gdt_ptr = {
        .len = (uint16_t) sizeof(early_gdt),
        .ptr = (uint32_t) &early_gdt
    };

    printk("GDT[1] Entry: 0x%l\n", early_gdt[1]);
    printk("GDT[2] Entry: 0x%l\n", early_gdt[2]);
    
    printk("Loading GDT\n");
    load_gdt(&gdt_ptr);

    printk("Reloading segments\n");
    reload_segments(0x08, 0x10);
}


void int_handler_c(void){
    uint8_t scancode = inb(0x60);
    printk("Got key: 0x%x\n", scancode);
}


extern void int_handler(void);
extern void default_handler(void);
void idt_setup(void)
{
     struct gate_desc early_idt[] __attribute__((aligned(16))) = {
//         [0 ... 255] = LDT_DESCRIPTOR_ENTRY( (uint32_t) int_handler, SELECTOR(0,0,1), 0x8e),
         [0 ... 255] = LDT_DESCRIPTOR_ENTRY( (uint32_t) default_handler, SELECTOR(0,0,1), 0x8e),
     };

//    early_idt[1].handler_addr0 = (uint16_t) ((uint32_t) int_handler) & 0xffff;
//    early_idt[1].handler_addr1 = (uint16_t) (((uint32_t) int_handler) >> 16);

     struct desc_table_ptr idt_ptr = {
         .len = (uint16_t) sizeof(early_idt),
         .ptr = (uint32_t) &early_idt
     };

//     idt_ptr.ptr = (uint32_t) &early_idt;
     
     printk("Loading IDT\n");
     load_idt(&idt_ptr);
}
