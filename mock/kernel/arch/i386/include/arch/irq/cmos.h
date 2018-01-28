#ifndef _CMOS_H
#define _CMOS_H

#define CMOS_IO_CMD         (0x70)
#define CMOS_IO_DATA        (0x71)
#define CMOS_DISABLE_NMI    (1 << 7)

/*
 * The CMOS landscape has use that wildly varies and remains, to this day,
 * non-standard. As such, we define here only the parts that _are_ standard,
 * or at least well-known, for our use from the OS
 */
#define RTC_SECOND      0x00
#define RTC_MINUTE      0x02
#define RTC_HOUR        0x04
#define RTC_WEEKDAY     0x06
#define RTC_DAY         0x07
#define RTC_MONTH       0x08
#define RTC_YEAR        0x09

#define CMOS_STATUS_A   0x0A
#define CMOS_STATUS_B   0x0B
#define CMOS_STATUS_C   0x0C
#define CMOS_STATUS_D   0x0D


/*
 * The bit-map associated with each of the status registers
 */
#define STATUSA_UPDATE_IN_PROGRESS (1 << 7)
#define STATUSA_FREQ_DIVISOR       (7 << 4)
#define STATUSA_RATE_SELECT_FREQ   (0x0f)

#define STATUSB_CLOCK_UPD_CYCLE    (1 << 7)
#define STATUSB_PERIODIC_INT       (1 << 6)
#define STATUSB_ALARM_INT          (1 << 5)
#define STATUSB_UPD_ENDED_INT      (1 << 4)
#define STATUSB_DATA_MODE          (1 << 3)
#define STATUSB_24HOUR_CLOCK       (1 << 2)
#define STATUSB_DAYLIGHT_SAVINGS   (1 << 1)

#define STATUSC_IRQ                (1 << 7)
#define STATUSC_PF                 (1 << 6)
#define STATUSC_AF                 (1 << 5)
#define STATUSC_UF                 (1 << 4)


#ifndef __ASSEMBLER__
/* A structure representing a snapshot of the Real Time Clock (RTC) */
struct cmos_rtc_time { 
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t weekday;
    uint8_t day;
    uint8_t month;
    uint8_t year;
};


void cmos_get_time(struct cmos_rtc_time *rtc);
void cmos_set_time(struct cmos_rtc_time *rtc);
uint8_t cmos_read_register(int reg);
void cmos_init(void);
void int_entry_cmos(void);

#endif  /* __ASSEMBLER__ */
#endif /* _CMOS_H */
