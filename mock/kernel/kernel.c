#include <mock.h>
#include <irq.h>
#include <arch/io.h>

extern uint32_t time_get_systick(void);
extern void cmos_test_removeme(void);

void kernel_main(void)
{
    printk("\n[%s] \n", __FUNCTION__);
    plat.irq_enable(0);
    plat.irq_enable(1);
    plat.irq_enable(2);
    plat.irq_enable(8);

    plat.irq_global_enable();

    while(1){
        cmos_test_removeme();
//        printk("\rTime: %d:%d:%d", hour, minute, second);
//        printk("\rsystick: 0x%x", time_get_systick());
    }
}
