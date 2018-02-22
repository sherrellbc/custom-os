.intel_syntax noprefix
#.org 0x7c00

#define RML_STACK   0x2000

.section .text
.code16

jmp _start

/*
 * Disk Address Packet
 * 
 * struct {
 *     uint8_t size;
 *     uint8_t pad; 
 *     uint16_t block_count;
 *     uint16_t offset;
 *     uint16_t segment;
 *     uint32_t lba_lower;
 *     uint32_t lba_upper;
 * }
 */
.align 4
da_packet:
    .byte 0x10      // Size of this structure
    .byte 0x00      // Zero
da_blk_cnt:
    .word 0x0000    // Number if blocks to read
da_buf:
    .word 0x0000    // Offset
    .word 0x0000    // Segment

da_lba_addr:
    .long 0x00000000 // Logical Block Address (LBA) to read (lower)
    .long 0x00000000 // Logical Block Address (LBA) to read (upper)


_start:

    /* Let's get everything into a known state */
    pushf
    cli
    xor ax, ax
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov sp, REALMODE_LOADER_STACK 
    popf
    sti

#    lea si, da_packet
    mov si, 0x7c04
    mov ah, 0x42
    mov dl, 0x80
    int 0x13

rm_entry:
    mov ax, 0xff
    mov bx, 0xff
    jmp .
