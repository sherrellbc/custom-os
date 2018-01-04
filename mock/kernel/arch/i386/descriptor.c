#include <mock.h>
#include <arch/descriptor.h>
#include <arch/irq.h>

#include <arch/io.h>


/*
 * Protected-mode tables. This structure contains data necessary to support
 * a protected-mode runtime environment (e.g. IDT/GDT)
 */
struct pm_tables { 
    struct gate_desc idt[256] __attribute__((aligned(16)));
    struct desc_table_ptr idt_ptr; 

    struct segment_desc gdt[256] __attribute__((aligned(16)));
    struct desc_table_ptr gdt_ptr;
} g_pm_tables = {0};


uint64_t dt_create_descriptor(uint32_t base, uint32_t limit, uint16_t flags)
{
    uint64_t desc = 0;

    desc  = ((base & 0xffff) << 16) | (limit & 0xffff);
    desc |= (base & 0xff000000) | ((flags & 0xd0ff) << 8) | (limit & 0xf0000) | (base & 0xff0000 >> 16);

    return desc;
}


void load_segments(uint16_t seg_code, uint16_t seg_data)
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
    g_pm_tables.gdt[0] = GDT_DESCRIPTOR_ENTRY(0,0,0);                      /* First required empty/null descriptor for error detection */
    g_pm_tables.gdt[1] = GDT_DESCRIPTOR_ENTRY(0, 0xfffff, (GDT_CODE_PL0)); /* Next two entries are for kernel space; DPL=0 */
    g_pm_tables.gdt[2] = GDT_DESCRIPTOR_ENTRY(0, 0xfffff, (GDT_DATA_PL0));
    g_pm_tables.gdt[3] = GDT_DESCRIPTOR_ENTRY(0, 0xfffff, (GDT_CODE_PL3)); /* Below two entries are for userspace; DPL=3 */
    g_pm_tables.gdt[4] = GDT_DESCRIPTOR_ENTRY(0, 0xfffff, (GDT_DATA_PL3));

    g_pm_tables.gdt_ptr = (struct desc_table_ptr) {
        .len = (uint16_t) sizeof(g_pm_tables.gdt)/sizeof(struct segment_desc),
        .ptr = (uint32_t) &g_pm_tables.gdt
    };

//    printk("GDT[1] Entry: 0x%l\n", g_pm_tables.gdt[1]);
//    printk("GDT[2] Entry: 0x%l\n", g_pm_tables.gdt[2]);

    load_gdt(&g_pm_tables.gdt_ptr);
    load_segments(0x08, 0x10);
    printk("Loaded GDT\n");
}


int gdt_set_slot(int slot, struct segment_desc *entry, int reload)
{
    if( (NULL != entry) && ((unsigned)slot < sizeof(g_pm_tables.gdt)/sizeof(struct segment_desc)) ){
        memcpy(&g_pm_tables.gdt[slot], entry, sizeof(struct segment_desc));
    }

    if(reload){
        //FIXME: get current segments, reload
        load_segments(0x08, 0x10);
    }

    return 0;
}


int gdt_get_slot(int slot, struct segment_desc *dst)
{
    if( (NULL != dst) && ((unsigned)slot < sizeof(g_pm_tables.gdt)) ){
        memcpy(dst, &g_pm_tables.gdt[slot], sizeof(struct segment_desc));
        return 0;
    }   

    return -1;
}


void idt_setup(void)
{
    /* Construct the initial IDT with entries all pointing to the default handler */
    for(int i=0; i<(int) (sizeof(g_pm_tables.idt)/sizeof(struct gate_desc)); i++){
        g_pm_tables.idt[i] = LDT_DESCRIPTOR_ENTRY(default_handler, SELECTOR(0,0,1), 0x8e);
    }

    g_pm_tables.idt_ptr = (struct desc_table_ptr) {
       .len = (uint16_t) sizeof(g_pm_tables.idt),
       .ptr = (uint32_t) &g_pm_tables.idt
    };
     
    load_idt(&g_pm_tables.idt_ptr);
    printk("Loaded IDT\n");
}


/* Isolate the functions that are dependent on what bit-length the kernel was compiled as */
//inline void *idt_entry_get_handler(struct gate_desc *entry)
//{
//    return ((entry.handler1 << 16) | (entry.handler0));
//}


int idt_set_slot(int slot, struct gate_desc *entry)
{

    /* Check that the request slot exists within the inserted IDT */
    if( (NULL != entry) && ((unsigned)slot < sizeof(g_pm_tables.idt)/sizeof(struct gate_desc)) ){
        memcpy(&g_pm_tables.idt[slot], entry, sizeof(struct gate_desc));
        /*
         *  If the passed handler was NULL (i.e. remove), hotswap in the default handler so we can 
         *  detect if the interrupt fires at a later time 
         */
        void *handler = (void*) ((entry->handler_addr1 << 16) | entry->handler_addr0);
        if(NULL == handler){
            //set to default
//            g_pm_tables.idt[slot].handler0 = (uint16_t) (handler
        }

        return 0;
    }

    return 0;
}


int idt_get_slot(int slot, struct gate_desc *dst)
{
    if( (NULL != dst) && ((unsigned)slot < sizeof(g_pm_tables.idt)/sizeof(struct gate_desc)) ){
        memcpy(dst, &g_pm_tables.idt[slot], sizeof(struct gate_desc));
        return 0;
    }   

    return -1;
}
