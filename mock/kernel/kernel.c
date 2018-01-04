#include <mock.h>
#include <irq.h>

extern uint64_t time_get_systick(void);
extern void time_delay_msec(uint32_t msec);

void kernel_main(void)
{
    printk("\n[%s] \n", __FUNCTION__);
    plat.irq_enable(0);
    plat.irq_enable(1);
    plat.irq_global_enable();

    while(1){
        printk("\rsystick: 0x%x", time_get_systick());
        time_delay_msec(1000);
    }
}
