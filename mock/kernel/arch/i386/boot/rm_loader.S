.intel_syntax noprefix
#.org 0x7c00

.section .text
.code16
start:
    jmp .
    mov ax, 0x12
    int 0x10

    mov ah, 0x0e
    mov al, 0x41
    xor bx, bx
    mov bl, 3
    xor cx, cx
    int 0x10
#    jmp 0x0000:rm_entry

rm_entry:
    mov ax, 0xff
    mov bx, 0xff
    jmp .

dead_loop:
#    hlt
    jmp _exit_loop

