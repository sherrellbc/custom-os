.intel_syntax noprefix

.global g_systick
.global time_systick_handler
.extern pic8259_eoi


/* 
 * This handler's sole responsibility is to increment 
 * the global systick counter for use in timekeeping
 */
.section .text
time_systick_handler:
    push ebx   
    push eax

    lea ebx, g_systick
    inc dword ptr [ebx] //What if quad?

    call pic8259_eoi 
    pop eax
    pop ebx
    iret
