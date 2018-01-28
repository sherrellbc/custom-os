#ifndef _ARCH_X86_IRQ_H
#define _ARCH_X86_IRQ_H

#include <stdint.h>
#include "irq/time.h"
#include "irq/cmos.h"
#include "irq/keyboard.h"
#include "irq/spurious.h"
#include <irq.h>


/* 
 * The default interrupt handler vector 
 */
irq_handler_t *g_int_default_vect;

/*
 * Enumeration of the types of interrupt controller 
 * hardware available in the x86 chipset
 */
enum irq_hw {
    IRQ_HW_PIC8259,
    IRQ_HW_APIC,
    IRQ_HW_xAPIC,
    IRQ_HW_x2APIC,
};

/*
 * Enumeration of the (protected mode) x86 exceptions and interrupts in
 * order of how they are defined in the specification. 
 */
enum x86_pm_exceptions {
    EXC_DIVIDE_ERROR            = 0,
    EXC_DEBUG_EXCEPTION         = 1,
    EXC_NMI                     = 2,
    EXC_BREAKPOINT              = 3,
    EXC_OVERFLOW                = 4,
    EXC_BOUND_RANGE_EXCEEDED    = 5,
    EXC_INVALID_OPCODE          = 6,
    EXC_DEVICE_NOT_AVAIL        = 7,
    EXC_DOUBLE_FAULT            = 8,
    EXC_COPROC_SEG_OVERRUN      = 9,
    EXC_INVALID_TSS             = 10,
    EXC_SEGMENT_NOT_PRESENT     = 11,
    EXC_STACK_SEGMENT_FAULT     = 12,
    EXC_GENERAL_PROTECTION_FAULT= 13,
    EXC_PAGE_FAULT              = 14,
    EXC_INTEL_RESERVED          = 15,
    EXC_FLOATING_POINT_ERROR    = 16,
    EXC_ALIGNMENT_CHECK         = 17,
    EXC_MACHINE_CHECK           = 18,
    EXC_SIMD_FLOATING_POINT     = 19,
    EXC_VIRT                    = 20
};

/*
 * Early interrupt initialization 
 */
void irq_init(void);


#endif /* _ARCH_X86_IRQ_H */
