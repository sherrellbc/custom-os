#include <mock.h>
#include <arch/irq.h>
#include <arch/io.h>

//TODO: Disable NMI while programming the RTC

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

#define BCD_TO_BINARY(bcd)  (10*((bcd & 0xf0) >> 4) + (bcd & 0x0f))

#define IS_RTC_REGISTER(reg)     (reg <= RTC_YEAR)

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
struct cmos_rtc_time g_latest_time = {0};


/* 
 * A global structure for caching (optimizing) queries of CMOS status.
 * Tracking this data prevents needless slow access to CMOS registers
 */
struct cmos_conf { 
    /* Flag indicating an asynchronous update to cmos time was performed */
    int async_update; 

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
        g_cmos_conf.async_update = 1;

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
    
    g_cmos_conf.async_update = 1;
    cmos_interrupt_set(cmos_int_orig);
    return MIN(len, CMOS_MEMORY_SIZE - offset);
}


int cmos_write_block(uint8_t *buf, size_t len, size_t offset)
{
    int cmos_int_orig = cmos_interrupt_set(0);

    for(int i=offset; i<(int)MIN(offset + len, CMOS_MEMORY_SIZE); i++){
        cmos_write_register(i, buf[i]);
    }

    g_cmos_conf.async_update = 1;
    cmos_interrupt_set(cmos_int_orig);
    return MIN(len, CMOS_MEMORY_SIZE - offset);
}


void cmos_set_time(struct cmos_rtc_time *rtc)
{
    int cmos_int_orig;

    if(NULL != rtc){
        cmos_int_orig = cmos_interrupt_set(0);

        cmos_write_register(RTC_SECOND, rtc->seconds);
        cmos_write_register(RTC_MINUTE, rtc->minutes);
        cmos_write_register(RTC_HOUR, rtc->hours);
        cmos_write_register(RTC_WEEKDAY, rtc->weekday);
        cmos_write_register(RTC_DAY, rtc->day);
        cmos_write_register(RTC_MONTH, rtc->month);
        cmos_write_register(RTC_YEAR, rtc->year);

        cmos_interrupt_set(cmos_int_orig);
    } 
}


void cmos_get_time(struct cmos_rtc_time *rtc)
{
    int cmos_int_orig;

    if(NULL != rtc){
        cmos_int_orig = cmos_interrupt_set(0);

        rtc->seconds =  cmos_read_register(RTC_SECOND);
        rtc->minutes =  cmos_read_register(RTC_MINUTE);
        rtc->hours =    cmos_read_register(RTC_HOUR);
        rtc->weekday =  cmos_read_register(RTC_WEEKDAY);
        rtc->day =      cmos_read_register(RTC_DAY);
        rtc->month =    cmos_read_register(RTC_MONTH);
        rtc->year =    cmos_read_register(RTC_YEAR);

        cmos_interrupt_set(cmos_int_orig);
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


//TODO: Configurable initialization
void cmos_init(void)
{
    /* Initialize the global CMOS/RTC configuration cache */
    cmos_init_conf();

    /* Initial conditions to ensure the handler updates these on the first execution */
    g_latest_time.seconds = 0xff;
    g_latest_time.minutes = 0xff;
    g_latest_time.hours =   0xff;
    g_latest_time.day =     0xff;
    g_latest_time.month =   0xff;
    g_latest_time.year =    0xff;

    /* 
     * Unmask the CMOS' "time updated" interrupt; it can be gated through the 
     * IDT as desired
     */
    //TODO: install handler if interrupts requested on
    cmos_interrupt_enable();
}


static void cmos_update_all(void)
{
    g_latest_time.seconds = cmos_read_register(RTC_SECOND);
    g_latest_time.minutes = cmos_read_register(RTC_MINUTE);
    g_latest_time.hours =   cmos_read_register(RTC_HOUR);
    g_latest_time.weekday = cmos_read_register(RTC_WEEKDAY);
    g_latest_time.day =     cmos_read_register(RTC_DAY);
    g_latest_time.month =   cmos_read_register(RTC_MONTH);
    g_latest_time.year =    cmos_read_register(RTC_YEAR);
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
#if 0
    uint8_t seconds, minutes, hours, day, month;

    /* 
     * Efficiently update the cmos time unless the RTC was 
     * asynchronously manually updated 
     */
    //XXX: Data formats
    if(0 == g_cmos_conf.async_update){
        seconds = cmos_read_register(RTC_SECOND);

        /* Seconds overflowed, update minutes */
        if(seconds < g_latest_time.seconds){
            g_latest_time.seconds = seconds;
            minutes = cmos_read_register(RTC_MINUTE);
            
            /* Minutes overflowed, update hours */
            if(minutes < g_latest_time.minutes){
                g_latest_time.minutes = minutes;
                hours = cmos_read_register(RTC_HOUR);                

                /* Hours overflowed, update days */
                if(hours < g_latest_time.hours){
                    g_latest_time.hours = hours;
                    day = cmos_read_register(RTC_DAY);

                    /* Day overflowed, update month */
                    if(day < g_latest_time.day){
                        g_latest_time.day = day;
                        month = cmos_read_register(RTC_MONTH);

                        /* Month overflowed, update year */
                        if(month < g_latest_time.month){
                            g_latest_time.month = month;
                            g_latest_time.year = cmos_read_register(RTC_YEAR);
                        }

                    }
                }

            }
        }else{
            g_latest_time.seconds = seconds;
        }
    }else{
        printk("Async update!\n");           
        cmos_update_all();
        g_cmos_conf.async_update = 0;
    }
#endif

    cmos_update_all();

    /* Now that we've collected the data, check if we need to compute conversions */
    switch(g_rtc_conf.time_data_mode){
        case RTC_TIME_FORMAT_BCD:
            g_latest_time.seconds = BCD_TO_BINARY(g_latest_time.seconds);
            g_latest_time.minutes = BCD_TO_BINARY(g_latest_time.minutes); 
            g_latest_time.hours =   BCD_TO_BINARY(g_latest_time.hours);
            break;

        case RTC_TIME_FORMAT_BINARY:
        default:
            break;
    }

    cmos_interrupt_clear();
}


//XXX: Delete me
void cmos_test_removeme(void)
{
    printk("\rWeekday:%d Time %d:%d:%d Date: %d/%d/%d             ",
            g_latest_time.weekday, 
            g_latest_time.hours, g_latest_time.minutes, g_latest_time.seconds,
            g_latest_time.day, g_latest_time.month, g_latest_time.year);
}
