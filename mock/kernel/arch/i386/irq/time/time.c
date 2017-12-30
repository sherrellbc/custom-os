#include <kernel/kernel.h>
#include <kernel/printk.h>

volatile uint32_t g_systick = 0;


uint32_t time_get_systick(void)
{
    return  g_systick;
}

void time_delay_msec(uint32_t msec)
{
    uint32_t ctime = g_systick;
    int ticks = msec/54.5;

    //FIXME: take into account the current tick resolution
    while(g_systick < (ctime + ticks))
        ;

    //FIXME: rollover
}
    
