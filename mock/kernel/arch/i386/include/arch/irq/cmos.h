#ifndef _CMOS_H
#define _CMOS_H

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
void cmos_update_handler_asm(void);
void cmos_init(void);



#endif /* _CMOS_H */
