.intel_syntax noprefix

.global int_handler   # Make global to be seen at link-time
.global default_handler
.extern int_handler_c
.extern kpanic


# Useful constants
.section .data
g_panic_str: .asciz "\nUnhandled interrupt %d\n"


.section .text

#
# The default interrupt handler for unimplemented interrupts. This handleer
# exists solely to panic the kernel and halt the system
# 
default_handler:
    # Read the current ISR from the PIC
    mov al, 0x0b
    outb 0x20, al
    #outb 0xa0, al

    xor eax, eax
    inb al, 0x20
#    inb bx, 0xa0
    push eax

    lea eax, g_panic_str
    push eax

    call kpanic     # void kpanic(const char *str, int) 
    hlt
    jmp .

int_handler:
    pushad
    
    call int_handler_c

    # Send EOI to the PIC
    mov al, 0x20
    out 0x20, al

    popad
    iret
