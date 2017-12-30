.intel_syntax noprefix

.global kb_handler_entry
.extern kb_handler
.extern pic8259_eoi

.section .text
kb_handler_entry:
    pushad
    
    call kb_handler
    call pic8259_eoi

    popad
    iret
