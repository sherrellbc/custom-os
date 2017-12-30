#include <kernel/printk.h>
#include <arch/io.h>


void kb_handler(void){ 
    int num = 0; 
    
    while(inb(0x64) & 1){ 
        printk("KB: 0x%x [%d]\n", inb(0x60), num++); 
    } 
}   
