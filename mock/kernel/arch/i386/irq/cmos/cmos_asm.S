.intel_syntax noprefix

#include <arch/irq/cmos.h>

/* Misc supporting external functions */
.extern pic8259_eoi

/* Various C handlers for CMOS interrupts */
.extern cmos_update_hdlr
.extern cmos_alarm_hdlr
.extern cmos_periodic_hdlr

.global int_entry_cmos

/*
 * This handler entry is shared by all types of interrupts
 * that may be generated from the CMOS. Notably, this CMOS
 * can generate alarm, periodic, and update interrupts. This
 * entry is responsible for determining the interrupt source
 * and delegating as appropriate. This is accomlished by reading
 * status register C and determining which bit is set. The
 * operation of reading this register also acknowledges and clears
 * this status flag enabling further interrupts to occur
 *
 * (Bit 4) 0x10 - Update
 * (Bit 5) 0x20 - Alarm
 * (Bit 6) 0x40 - Periodic
 */
.section .text
int_entry_cmos:
    pushad
    
    mov al, CMOS_STATUS_C
    outb CMOS_IO_CMD
    inb al, CMOS_IO_DATA
    mov bl, al

check_periodic:
    and al, STATUSC_PF
    jz check_update
    
    call cmos_periodic_hdlr
    jmp ack_int

check_update:
    mov al, bl
    and al, STATUSC_UF
    jz check_alarm

    call cmos_update_hdlr
    jmp ack_int

check_alarm:
    call cmos_alarm_hdlr


ack_int:
    call pic8259_eoi
    popad
    iret
