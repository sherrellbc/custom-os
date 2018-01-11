.intel_syntax noprefix
.altmacro

.global g_int_default_vect
.extern pic8259_eoi

/* 
 * A simple macro for defining an interrupt entry 
 * to pushes its interrupt number before calling
 * the common entry
 */
.macro irq_stubX number
    irq\number:
    pushd \number
    jmp default_handler_common
.endm

/*
 * A macro for creating the interrupt entry point stubs and
 * placing (contiguous) entries in the .data section when
 * used in a loop
 */
.macro irq_insertX number
    .section .text
    irq_stubX \number

    .section .data
    .long irq\number 
.endm



.section .data
unhandled_int_string: 
    .asciz "\n\n*** Unhandled interrupt [%d] ***\n"

g_int_default_vect:
    .long default_handlers


.section .text
default_handler_common:
    pushad

    /* Move past everything from pushad (8*4 bytes) */
    push  [esp + 4*8] 
    lea eax, unhandled_int_string
    push eax
    call printk
    pop eax
    pop eax

    call pic8259_eoi

    popad

do_halt:
    hlt
    jmp do_halt



/* Using the macro above, create 256 default interrupt entries */
.section .data
default_handlers:
.set i, 0
.rept 256
    irq_insertX %i 
    .set i, i+1
.endr
