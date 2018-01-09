#include <kernel/kernel.h>
#include <mock.h>
#include <platform.h>
#include <irq.h>

extern uint32_t time_get_systick(void);

void kernel_main(void)
{
    printk("\n[%s] \n", __FUNCTION__);
//    plat.irq_enable(0);
    plat.irq_enable(1);
    plat.irq_global_enable();

    while(1){
//        printk("\rsystick: 0x%x", time_get_systick());
    }
}
