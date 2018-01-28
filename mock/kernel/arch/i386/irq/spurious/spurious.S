.intel_syntax noprefix
.extern pic8259_eoi
.extern pic8259_getmask
.extern printk
.global int_entry_spurious
.extern pic8259_get_irr
.extern pic8259_get_isr


//FIXME: Remove the isr/irr dependency once testing is complete
.section .data
g_spr_count:    .long 0
#g_spr_alert:    .asciz "\n\n*** Spurious interrupt [%d], mask=0x%x ***\n"
g_spr_alert:    .asciz "\n\n*** Spurious interrupt [%d], mask=0x%x, isr=0x%x, irr=0x%x ***\n"


.section .text
int_entry_spurious:
    pushad

    call pic8259_get_irr
    push eax

    call pic8259_get_isr
    push eax

    call pic8259_getmask
    push eax

    /* Push the current count and increment it */
    mov eax, g_spr_count
    push eax
    inc eax
    mov g_spr_count, eax

    lea eax, g_spr_alert
    push eax
    call printk
    add esp, 20

    call pic8259_eoi
    popad
    iret
