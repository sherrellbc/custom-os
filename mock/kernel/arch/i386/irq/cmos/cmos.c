#include <mock.h>
#include <arch/irq.h>
#include <arch/io.h>

//TODO: Disable NMI while programming the RTC
//TODO: Use of the RTC timers/alarm interrupts
//TODO: More configurable cmos init (interrupts, clock rate, etc)
//TODO: Fix the information returned from cmos_int_* functions
//TODO: Reimplement as "time since boot" and use time in nanoseconds
//TODO: Start with higher-level "time" subsystem that initializes this

/*
 * Theory
 *
 * The CMOS is a legacy periperhal that eventually found itself
 * integrated into the Intel chipset to maintain compatibility with the
 * early IBM PC-AT. This peripheral is truly a hodge-podge of historically
 * useful components all thrown into one IO-mapped device. Fundamentally, 
 * the CMOS contains 128 bytes of battery-backed non-volatile memory
 * at IO addresses 0x70 and 0x71. Some vendors have also decided to map
 * a mirror copy of the CMOS registers to 0x72 and 0x73. 0x74 was later
 * consumed (on Intel designs) to behave as a read-only register that 
 * allowed the contents of the last written value to be read back. This
 * feature is useful with particular emphasis in the Non-Maskable
 * Interrupt (NMI) status. Historically the CMOS has been used
 * by firmware vendors to store non-volatile information across
 * power cycles. This enabled firmware to maintain simple state information
 * to provide a richer environment for users. Of course, as it happened, 
 * the information contained within the CMOS is entirely vendor/implementation
 * dependent and is not standardized (for the most part). However, though
 * non-standard, some offsets into the CMOS are so commonly used for
 * a particular purpose that they can nearly be considered so. One very
 * important consideration when dealing with the CMOS is that the gating
 * bit for the NMI was mapped into the MSB of the address used to select
 * an internal CMOS register. Thus, the CMOS registers are effectively
 * mirrored for [0x00 - 0x7f] and [0x80 - 0xff]. This works because the
 * MSB of the register address was strictly used to gate the NMI while
 * only the lower 7 bits were used to form the address.
 *
 * Altogether the CMOS contains:
 *      Real Time Clock (RTC)
 *      Information about installed floppy drivers
 *      Power state information
 *      Various system configuration settings
 *      Hard disk types installed
 *      Misc other installed devices
 *      Bounds of installed memory
 *      ...
 *
 * A good reference for some of the data that can be found in the CMOS map can 
 * be found here: http://www.bioscentral.com/misc/cmosmap.htm
 */


#define CMOS_MEMORY_SIZE        (128)

#define BCD_TO_BINARY(bcd)      (10*(((bcd) & 0xf0) >> 4) + ((bcd) & 0x0f))
#define BINARY_TO_BCD(bin)      ((((bin)/10) << 4) | ((bin) % 10))
#define IS_RTC_REGISTER(reg)    ((reg) <= RTC_YEAR)


/*
 * An asynchronously-updated globally accessible variable for storing
 * the latest time read from the CMOS/RTC
 */
struct cmos_rtc_time g_time_hw = {0};


/* A constant data structure used to compute date transitions relating to day and month rollovers */
uint8_t const g_month_daycount[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};


/* 
 * A global structure for caching (optimizing) queries of CMOS status.
 * Tracking this data prevents needless slow access to CMOS registers
 */
struct cmos_conf { 
    /* Flag indicating the state of the CMOS "time updated" interrupt */
    int int_status;
}g_cmos_conf;


/*
 * Configuration globals representing the determined state of the CMOS/RTC's data format
 */
enum rtc_time_format { 
    RTC_TIME_FORMAT_BINARY,
    RTC_TIME_FORMAT_BCD,
    RTC_TIME_24_HOUR,
    RTC_TIME_12_HOUR,
};

struct rtc_conf {
    uint8_t time_data_mode;
    uint8_t time_hour_format;
    uint8_t daylight_savings;

    /* Flag indicating an asynchronous update to cmos time was performed */
    int async_update; 
} g_rtc_conf;


/* 
 * Enable CMOS interrupts
 * @return  The state of the interrupt flag before enabling
 */
int cmos_int_enable(int interrupt)
{
    uint8_t status_b;

    outb(CMOS_IO_CMD, CMOS_STATUS_B);
    status_b = inb(CMOS_IO_DATA);

    /* We must reselect register B; the above read resets the index to D */
    outb(CMOS_IO_DATA, status_b | (uint8_t)interrupt);

    g_cmos_conf.int_status = (0 != (status_b & (uint8_t)interrupt));
    return g_cmos_conf.int_status;
}


/* 
 * Disable CMOS interrupts
 * @return  The state of the interrupt flag before disabling
 */
int cmos_int_disable(int interrupt)
{
    uint8_t status_b;

    outb(CMOS_IO_CMD, CMOS_STATUS_B);
    status_b = inb(CMOS_IO_DATA);

    /* We must reselect register B; the above read resets the index to D */
    outb(CMOS_IO_CMD, CMOS_STATUS_B);
    outb(CMOS_IO_DATA, status_b & ~((uint8_t)interrupt));

    g_cmos_conf.int_status = (0 != (status_b & (uint8_t)interrupt));
    return g_cmos_conf.int_status;
}


uint8_t cmos_reg_read(int reg)
{
    outb(CMOS_IO_CMD, reg);
    return inb(CMOS_IO_DATA);
}


void cmos_reg_write(int reg, uint8_t data)
{
    int irq_orig;

    /* 
     * If the write targets an RTC time register we need to first
     * disable interrpts followed by setting a global flag to inform
     * the RTC handler that some time data has changed and it needs to 
     * update the stored cache
     */
    if(IS_RTC_REGISTER(reg)){    
        irq_orig = plat.irq_set(8, 0);        

        outb(CMOS_IO_CMD, reg);
        outb(CMOS_IO_DATA, data);
        g_rtc_conf.async_update = 1;

        plat.irq_set(8, irq_orig);        
        return;
    }

    outb(CMOS_IO_CMD, reg);
    outb(CMOS_IO_DATA, data);
}


int cmos_read_block(uint8_t *buf, size_t len, size_t offset)
{
    int irq_orig = plat.irq_set(8, 0);

    for(int i=0; i<(int)MIN(offset + len, CMOS_MEMORY_SIZE); i++){
        buf[i] = cmos_reg_read(i);
    }
    
    g_rtc_conf.async_update = 1;
    plat.irq_set(8, irq_orig);
    return MIN(len, CMOS_MEMORY_SIZE - offset);
}


int cmos_write_block(uint8_t *buf, size_t len, size_t offset)
{
    int irq_orig = plat.irq_set(8, 0);

    for(int i=offset; i<(int)MIN(offset + len, CMOS_MEMORY_SIZE); i++){
        cmos_reg_write(i, buf[i]);
    }

    g_rtc_conf.async_update = 1;
    plat.irq_set(8, irq_orig); 
    return MIN(len, CMOS_MEMORY_SIZE - offset);
}


void cmos_set_time(struct cmos_rtc_time *time)
{
    struct cmos_rtc_time *tmp_time = time;
    struct cmos_rtc_time adjusted;
    int irq_orig;

    if(NULL != time){
        /* 
         * We need to preprocess the received time if the 
         * hardware's state is configured to require BCD formatted
         * time data
         */
        if(RTC_TIME_FORMAT_BCD & g_rtc_conf.time_data_mode){
            
            adjusted.seconds =  BINARY_TO_BCD(time->seconds);
            adjusted.minutes =  BINARY_TO_BCD(time->minutes); 
            adjusted.hours =    BINARY_TO_BCD(time->hours);
            adjusted.day =      BINARY_TO_BCD(time->day);
            adjusted.weekday =  BINARY_TO_BCD(time->weekday);
            adjusted.month =    BINARY_TO_BCD(time->month);
            adjusted.year =     BINARY_TO_BCD(time->year);

            tmp_time = &adjusted;
        }

        irq_orig = plat.irq_set(8, 0);

        /* 
         * Doing IO can take a relatively long time, so we discriminate
         * and only write to the CMOS if the data is different than our
         * current in-memory cache that tracks the state of hardware
         */
        if(g_time_hw.seconds != tmp_time->seconds){
            cmos_reg_write(RTC_SECOND, tmp_time->seconds);
        }

        if(g_time_hw.minutes != tmp_time->minutes){
            cmos_reg_write(RTC_MINUTE, tmp_time->minutes);
        }

        if(g_time_hw.hours != tmp_time->hours){
            cmos_reg_write(RTC_HOUR, tmp_time->hours);
        }

        if(g_time_hw.weekday != tmp_time->weekday){
            cmos_reg_write(RTC_WEEKDAY, tmp_time->weekday);
        }

        if(g_time_hw.day != tmp_time->day){
            cmos_reg_write(RTC_DAY, tmp_time->day);
        }

        if(g_time_hw.month != tmp_time->month){
            cmos_reg_write(RTC_MONTH, tmp_time->month);
        }

        if(g_time_hw.year != tmp_time->year){
            cmos_reg_write(RTC_YEAR, tmp_time->year);
        }

        plat.irq_set(8, irq_orig);
    } 
}


void cmos_get_time(struct cmos_rtc_time *time)
{
    int irq_orig;

    if(NULL != time){
        irq_orig = plat.irq_set(8, 0);
        memcpy(time, &g_time_hw, sizeof(struct cmos_rtc_time));
        plat.irq_set(8, irq_orig);

        /* Now that we've collected the data, check if we need to compute conversions */
        if(RTC_TIME_FORMAT_BCD & g_rtc_conf.time_data_mode){
            time->seconds =  BCD_TO_BINARY(time->seconds);
            time->minutes =  BCD_TO_BINARY(time->minutes); 
            time->hours =    BCD_TO_BINARY(time->hours);
            time->day =      BCD_TO_BINARY(time->day);
            time->weekday =  BCD_TO_BINARY(time->weekday);
            time->month =    BCD_TO_BINARY(time->month);
            time->year =     BCD_TO_BINARY(time->year);
        }
    }
}


static void cmos_init_conf(void)
{
    uint8_t status_b = cmos_reg_read(CMOS_STATUS_B);
    memset(&g_cmos_conf, 0, sizeof(g_cmos_conf));
    
    /* Status register B provides information on how the time/date data are formatted */
    (status_b & STATUSB_24HOUR_CLOCK) ? 
        (g_rtc_conf.time_hour_format = RTC_TIME_24_HOUR) : 
        (g_rtc_conf.time_hour_format = RTC_TIME_12_HOUR);

    (status_b & STATUSB_DATA_MODE) ? 
        (g_rtc_conf.time_data_mode = RTC_TIME_FORMAT_BINARY) : 
        (g_rtc_conf.time_data_mode = RTC_TIME_FORMAT_BCD);

    (status_b & STATUSB_DAYLIGHT_SAVINGS) ? 
        (g_rtc_conf.daylight_savings = 1) : 
        (g_rtc_conf.daylight_savings = 0);
}


static void cmos_sync_hw_time(void)
{
    g_time_hw.seconds = cmos_reg_read(RTC_SECOND);
    g_time_hw.minutes = cmos_reg_read(RTC_MINUTE);
    g_time_hw.hours =   cmos_reg_read(RTC_HOUR);
    g_time_hw.weekday = cmos_reg_read(RTC_WEEKDAY);
    g_time_hw.day =     cmos_reg_read(RTC_DAY);
    g_time_hw.month =   cmos_reg_read(RTC_MONTH);
    g_time_hw.year =    cmos_reg_read(RTC_YEAR);
}


void cmos_init(void)
{
    /* Initialize the global CMOS/RTC configuration cache */
    cmos_init_conf();

    /* Initial system time sync with hardware time */
    cmos_sync_hw_time();

    /* 
     * Unmask the CMOS' "time updated" interrupt; it can be gated through the 
     * IDT as desired
     */
    cmos_int_enable(STATUSB_UPD_ENDED_INT);
}


/*
 * Use this periodic interrupt to help keep track of system time without
 * incurring the large time overhead of actually reading from hardware
 */
void cmos_update_hdlr(void)
{
    g_time_hw.seconds = ((g_time_hw.seconds + 1) % 60);

    /* Seconds overflowed, check minutes */
    if(0 == g_time_hw.seconds){
        g_time_hw.minutes = ((g_time_hw.minutes + 1) % 60);

        /* Minutes overflowed, check hours */
        if(0 == g_time_hw.minutes){
            g_time_hw.hours = ((g_time_hw.hours + 1) % 24);

            /* Hours overflowed, check days */
            if(0 == g_time_hw.hours){
                g_time_hw.day = ((g_time_hw.day + 1) % g_month_daycount[g_time_hw.day]);
                g_time_hw.weekday = ((g_time_hw.weekday + 1) % 7);

                /* Days overflowed, check month */
                if(0 == g_time_hw.day){
                    g_time_hw.month = ((g_time_hw.month + 1) % 12);

                    /* Month overflowed, increment year */
                    g_time_hw.year++;
                }
            }

        }
    }
}


void cmos_alarm_hdlr(void)
{
    printk("\n[%s] Unimplemented\n", __FUNCTION__);
}


void cmos_periodic_hdlr(void)
{
    printk("\n[%s] Unimplemented\n", __FUNCTION__);
}


//XXX: Delete me
void cmos_test_removeme(void)
{
    static int count = 0;
    struct cmos_rtc_time time;

    if(0 == (++count % 400000)){
        memset(&time, 7, sizeof(time));
        cmos_set_time(&time);
    }

    cmos_get_time(&time);
    printk("\rWeekday:%d Time %d:%d:%d Date: %d/%d/%d [%d]          \r",
            time.weekday, 
            time.hours, time.minutes, time.seconds,
            time.month, time.day, time.year, count);
}
