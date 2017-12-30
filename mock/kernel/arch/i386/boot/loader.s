.intel_syntax noprefix
.set MAGIC,                 0x1badb002 
.set BOOT_MODULES_ALIGNED,  (1<<0)
.set MEMINFO,               (1<<1)  
.set VIDEOINFO,             (0<<2)  # Graphics fields of the multiboot header
.set FLAGS,                 (BOOT_MODULES_ALIGNED | MEMINFO | VIDEOINFO)
.set CHECKSUM,              -(MAGIC + FLAGS)

.extern __stack
.extern exit_panic 
.extern x86_boot_legacy # Extern to be seen from _this_ file at link-time
.global kernel_loader   # Made global to be seen at link-time


# GRUB(legacy/2) can boot mutliboot-compatible kernels; make us compatible
.section .multiboot
    .long MAGIC
    .long FLAGS
    .long CHECKSUM


/*
 * Per the spec
 *  - eax contains an adjusted magic
 *  - ebc contains a 32-bit pointer to the multiboot information structure
 */
.section .text
kernel_loader:
    lea esp, __stack
    push eax
    push ebx
    cli             # Disable interrupts
    call x86_boot_legacy 

_exit_loop:
    call exit_panic
    cli
    hlt
    jmp _exit_loop


.section .bss
