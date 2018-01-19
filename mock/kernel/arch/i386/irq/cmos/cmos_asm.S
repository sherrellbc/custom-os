.intel_syntax noprefix
.extern cmos_update_handler
.extern pic8259_eoi 
.global cmos_update_handler_asm

.section .text
cmos_update_handler_asm:
    pushad
    call cmos_update_handler
    call pic8259_eoi 
    popad
    iret
