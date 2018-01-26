#include <mock.h>
#include <arch/irq.h>
#include <arch/io.h>

//TODO: Disable NMI while programming the RTC
//TODO: Use of the RTC timers/alarm interrupts

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


#define CMOS_DISABLE_NMI    (1 << 7)

#define CMOS_MEMORY_SIZE    (128)
#define CMOS_IO_CMD         (0x70)
#define CMOS_IO_DATA        (0x71)

#define BCD_TO_BINARY(bcd)  (10*(((bcd) & 0xf0) >> 4) + ((bcd) & 0x0f))
#define BINARY_TO_BCD(bin)  ((((bin)/10) << 4) | ((bin) % 10))

#define IS_RTC_REGISTER(reg)     ((reg) <= RTC_YEAR)

/*
 * The CMOS landscape has use that wildly varies and remains, to this day,
 * non-standard. As such, we define here only the parts that _are_ standard,
 * or at least well-known, for our use from the OS
 */
enum cmos_map {
    RTC_SECOND      = 0x00,
    RTC_MINUTE      = 0x02,
    RTC_HOUR        = 0x04,
    RTC_WEEKDAY     = 0x06,
    RTC_DAY         = 0x07,
    RTC_MONTH       = 0x08,
    RTC_YEAR        = 0x09,

    CMOS_STATUS_A   = 0x0A,
    CMOS_STATUS_B   = 0x0B,
    CMOS_STATUS_C   = 0x0C,
    CMOS_STATUS_D   = 0x0D,
};


/*
 * The bit-map associated with each of the status registers 
 */
enum status_reg_masks {
    STATUSA_UPDATE_IN_PROGRESS_MASK = (1 << 7),
    STATUSA_FREQ_DIVISOR_MASK       = (7 << 4),
    STATUSA_RATE_SELECT_FREQ_MASK   = (0x0f),

    STATUSB_CLOCK_UPD_CYCLE_MASK    = (1 << 7),
    STATUSB_PERIODIC_INT_MASK       = (1 << 6),
    STATUSB_ALARM_INT_MASK          = (1 << 5),
    STATUSB_UPD_ENDED_INT_MASK      = (1 << 4),
    STATUSB_DATA_MODE_MASK          = (1 << 3),
    STATUSB_24HOUR_CLOCK_MASK       = (1 << 2),
    STATUSB_DAYLIGHT_SAVINGS_MASK   = (1 << 1),

    STATUSC_IRQ_FLAG_MASK           = (1 << 7),
    STATUSC_PF_FLAG_MASK            = (1 << 6),
    STATUSC_AF_FLAG_MASK            = (1 << 5),
    STATUSC_UF_FLAG_MASK            = (1 << 4),
};


/*
 * An asynchronously-updated globally accessible variable for storing
 * the latest time read from the CMOS/RTC
 */
struct cmos_rtc_time g_time_hw = {0};


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
int cmos_interrupt_enable(void)
{
    uint8_t status_b;

    outb(CMOS_IO_CMD, CMOS_STATUS_B);
    status_b = inb(CMOS_IO_DATA);

    /* We must reselect register B; the above read resets the index to D */
    outb(CMOS_IO_DATA, status_b | STATUSB_UPD_ENDED_INT_MASK);

    g_cmos_conf.int_status = (0 != (status_b & STATUSB_UPD_ENDED_INT_MASK));
    return g_cmos_conf.int_status;
}


/* 
 * Disable CMOS interrupts
 * @return  The state of the interrupt flag before disabling
 */
int cmos_interrupt_disable(void)
{
    uint8_t status_b;

    outb(CMOS_IO_CMD, CMOS_STATUS_B);
    status_b = inb(CMOS_IO_DATA);

    /* We must reselect register B; the above read resets the index to D */
    outb(CMOS_IO_CMD, CMOS_STATUS_B);
    outb(CMOS_IO_DATA, status_b & ~(STATUSB_UPD_ENDED_INT_MASK));

    g_cmos_conf.int_status = (0 != (status_b & STATUSB_UPD_ENDED_INT_MASK));
    return g_cmos_conf.int_status;
}


/* 
 * Set the state of CMOS interrupts
 * @return  The state of the interrupt flag before the operation
 */
static int cmos_interrupt_set(int state)
{
    if(0 == state){
        return cmos_interrupt_disable();
    }
    
    return cmos_interrupt_enable();
}


uint8_t cmos_read_register(int reg)
{
    outb(CMOS_IO_CMD, reg);
    return inb(CMOS_IO_DATA);
}


void cmos_write_register(int reg, uint8_t data)
{
    int cmos_int_orig;

    /* 
     * If the write targets an RTC time register we need to first
     * disable interrpts followed by setting a global flag to inform
     * the RTC handler that some time data has changed and it needs to 
     * update the stored cache
     */
    if(IS_RTC_REGISTER(reg)){    
        cmos_int_orig = cmos_interrupt_set(0);

        outb(CMOS_IO_CMD, reg);
        outb(CMOS_IO_DATA, data);
        g_rtc_conf.async_update = 1;

        cmos_interrupt_set(cmos_int_orig);
        return;
    }

    outb(CMOS_IO_CMD, reg);
    outb(CMOS_IO_DATA, data);
}


int cmos_read_block(uint8_t *buf, size_t len, size_t offset)
{
    int cmos_int_orig = cmos_interrupt_set(0);

    for(int i=0; i<(int)MIN(offset + len, CMOS_MEMORY_SIZE); i++){
        buf[i] = cmos_read_register(i);
    }
    
    g_rtc_conf.async_update = 1;
    cmos_interrupt_set(cmos_int_orig);
    return MIN(len, CMOS_MEMORY_SIZE - offset);
}


int cmos_write_block(uint8_t *buf, size_t len, size_t offset)
{
    int cmos_int_orig = cmos_interrupt_set(0);

    for(int i=offset; i<(int)MIN(offset + len, CMOS_MEMORY_SIZE); i++){
        cmos_write_register(i, buf[i]);
    }

    g_rtc_conf.async_update = 1;
    cmos_interrupt_set(cmos_int_orig);
    return MIN(len, CMOS_MEMORY_SIZE - offset);
}


void cmos_set_time(struct cmos_rtc_time *time)
{
    int cmos_int_orig;
    struct cmos_rtc_time *tmp_time = time;
    struct cmos_rtc_time adjusted;

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

        cmos_int_orig = cmos_interrupt_set(0);

        /* 
         * Write in reverse order to prevent any race conditions 
         * with the 'seconds' updating 
         */
        cmos_write_register(RTC_YEAR, tmp_time->year);
        cmos_write_register(RTC_MONTH, tmp_time->month);
        cmos_write_register(RTC_DAY, tmp_time->day);
        cmos_write_register(RTC_WEEKDAY, tmp_time->weekday);
        cmos_write_register(RTC_HOUR, tmp_time->hours);
        cmos_write_register(RTC_MINUTE, tmp_time->minutes);
        cmos_write_register(RTC_SECOND, tmp_time->seconds);

        cmos_interrupt_set(cmos_int_orig);
    } 
}


void cmos_get_time(struct cmos_rtc_time *time)
{
    int cmos_int_orig;

    if(NULL != time){
        cmos_int_orig = cmos_interrupt_set(0);
        memcpy(time, &g_time_hw, sizeof(struct cmos_rtc_time));
        cmos_interrupt_set(cmos_int_orig);

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
    uint8_t status_b = cmos_read_register(CMOS_STATUS_B);
    memset(&g_cmos_conf, 0, sizeof(struct cmos_conf));
    
    /* Status register B provides information on how the time/date data are formatted */
    (status_b & STATUSB_24HOUR_CLOCK_MASK) ? 
        (g_rtc_conf.time_hour_format = RTC_TIME_24_HOUR) : 
        (g_rtc_conf.time_hour_format = RTC_TIME_12_HOUR);

    (status_b & STATUSB_DATA_MODE_MASK) ? 
        (g_rtc_conf.time_data_mode = RTC_TIME_FORMAT_BINARY) : 
        (g_rtc_conf.time_data_mode = RTC_TIME_FORMAT_BCD);

    (status_b & STATUSB_DAYLIGHT_SAVINGS_MASK) ? 
        (g_rtc_conf.daylight_savings = 1) : 
        (g_rtc_conf.daylight_savings = 0);
}


static void cmos_sync_hw_time(void)
{
    g_time_hw.seconds = cmos_read_register(RTC_SECOND);
    g_time_hw.minutes = cmos_read_register(RTC_MINUTE);
    g_time_hw.hours =   cmos_read_register(RTC_HOUR);
    g_time_hw.weekday = cmos_read_register(RTC_WEEKDAY);
    g_time_hw.day =     cmos_read_register(RTC_DAY);
    g_time_hw.month =   cmos_read_register(RTC_MONTH);
    g_time_hw.year =    cmos_read_register(RTC_YEAR);
}



//TODO: Configurable initialization
void cmos_init(void)
{
    /* Initialize the global CMOS/RTC configuration cache */
    cmos_init_conf();

    /* 
     * Initial conditions to ensure the handler updates these on 
     * the first execution. Note that we are updating the 
     */
    memset(&g_time_hw, 0xff, sizeof(struct cmos_rtc_time));

    /* 
     * Unmask the CMOS' "time updated" interrupt; it can be gated through the 
     * IDT as desired
     */
    //TODO: install handler if interrupts requested on
    cmos_interrupt_enable();
}


/*
 * "Clearing" the interrupt requires only that the status byte containing
 * the interrupt flag be read. The contents are unimportant and discarded 
 */
static void cmos_interrupt_clear(void)
{
    outb(CMOS_IO_CMD, CMOS_STATUS_C);
    inb(CMOS_IO_DATA);
}


void cmos_update_handler(void)
{
    struct cmos_rtc_time htime;

    /* 
     * Efficiently update the cmos time unless the RTC was 
     * asynchronously manually updated 
     */
    if(0 == g_rtc_conf.async_update){
        htime.seconds = cmos_read_register(RTC_SECOND);

        /* Seconds overflowed, update minutes */
        if(htime.seconds < g_time_hw.seconds){
            g_time_hw.seconds = htime.seconds;
            htime.minutes = cmos_read_register(RTC_MINUTE);
            
            /* Minutes overflowed, update hours */
            if(htime.minutes < g_time_hw.minutes){
                g_time_hw.minutes = htime.minutes;
                htime.hours = cmos_read_register(RTC_HOUR);                

                /* Hours overflowed, update days */
                if(htime.hours < g_time_hw.hours){
                    g_time_hw.hours = htime.hours;
                    htime.day = cmos_read_register(RTC_DAY);

                    /* Day overflowed, update month */
                    if(htime.day < g_time_hw.day){
                        g_time_hw.day = htime.day;
                        htime.month = cmos_read_register(RTC_MONTH);

                        /* If the day rolled over, also update the weekday */
                        g_time_hw.weekday = cmos_read_register(RTC_WEEKDAY);

                        /* Month overflowed, update year */
                        if(htime.month < g_time_hw.month){
                            g_time_hw.month = htime.month;
                            g_time_hw.year = cmos_read_register(RTC_YEAR);
                        }

                    }
                }

            }
        }else{
            g_time_hw.seconds = htime.seconds;
        }
    }else{
        printk("Async update!\n");           
        cmos_sync_hw_time(); 
        g_rtc_conf.async_update = 0;
    }
    cmos_interrupt_clear();
}


//XXX: Delete me
void cmos_test_removeme(void)
{
    struct cmos_rtc_time time;
    cmos_get_time(&time);

    printk("\rWeekday:%d Time %d:%d:%d Date: %d/%d/%d             \r",
            time.weekday, 
            time.hours, time.minutes, time.seconds,
            time.month, time.day, time.year);
}
