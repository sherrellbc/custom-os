.intel_syntax noprefix

.global default_handler
.extern pic8259_get_register
.extern pic8259_eoi
#.extern kpanic

.section .data
g_panic_str: .asciz "Unhandled interrupt 0x%x\n"

# The default system handler. Get the interrupt number and kpanic
.section .text
default_handler:
    pushad

    mov eax, 0x0b
    push eax
    call pic8259_get_register

    push eax
    lea eax, g_panic_str
    push eax
    call printk

    pop eax
    pop eax
    pop eax

    call pic8259_eoi
    popad

do_halt:
    hlt
    jmp do_halt
#    kpanic
#    iret
