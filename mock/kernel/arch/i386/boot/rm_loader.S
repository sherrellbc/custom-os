.intel_syntax noprefix
#.org 0x7c00

//TODO: Add prints to the console for status
//TODO: Support machines without LBA extensions (or at least print)

#define RML_STACK       0x2000
#define RML_RELOC_ADDR  0x0600

.section .text
.code16

jmp _start

/*
 * Disk Address Packet
 * 
 * struct {
 *     uint8_t  size;
 *     uint8_t  pad; 
 *     uint16_t block_count;
 *     uint16_t offset;
 *     uint16_t segment;
 *     uint32_t lba_lower;
 *     uint32_t lba_upper;
 * }
 */
.align 2
da_packet:
    .byte 0x10      // Size of this structure
    .byte 0x00      // Zero
da_blk_cnt:
    .word 0x0000    // Number of blocks to read
da_buf:
    .word 0x0000    // Offset
    .word 0x0000    // Segment

da_lba_addr:
    .long 0x00000000 // Logical Block Address (LBA) to read (lower)
    .long 0x00000000 // 64-bit flat address of trasnfer buffer (used if dword at 0x04 is ffff:ffff and EDD-3.0)


/*
 * Query the BIOS for important information.
 * Read the first sector of disk
 * Parse the MBR table to determine the "Active" partition
 * Load the VBR from this partition and jump there
 */
_start:
    /* Let's get everything into a known state */
    mov sp, RML_STACK 
    xor ax, ax
    mov ss, ax
    mov ds, ax
    mov es, ax

    /* Then relocate ourselves */
    mov si, 0x7c00
    mov di, RML_RELOC_ADDR
    mov cx, 0x100
    rep movsw
    jmp 0x0,relocated_start // This works _only if_ we are linked at this relocated address

relocated_start:

    /* 
     * Check to see if the LBA/DA packet extensions exist. This
     * is currently the only case this MBR stage handles.
     * TODO: Use CHS from the MBR
     */
    call check_int13_extensions
    jnz no_int13_ext_support

    /* Looks like we're good. Load the first block from disk (Defaults assumed)*/

    /* The sector is in place. Parse the table and load the VBR */
    mov di, RML_RELOC_ADDR
    call load_vbr
    test ax,ax
    jnz error_loop

    /* The code is loaded; jump there */
    jmp 0x0,0x7c00
    

no_int13_ext_support:
    //Print characters
error_loop:
    cli
    hlt
    jmp error_loop

/*
 * Determine if the LBA extensions are suppored for the first disk (0x80)
 * ax = 0 (success) or non-zero (error) on return
 */
check_int13_extensions:
    mov ah, 0x41
    mov bx, 0x55aa
    mov dl, 0x80
    int 0x13
    jc err_or_unsupported
    cmp bx, 0xaa55
    jnz err_or_unsupported
    mov ax, 0
    jmp ext_determined

err_or_unsupported:
    mov ax, 0xffff
ext_determined:
    ret

/*
 * Uses the LBA extensions to read from the first disk (0x80).
 * si = address of DA packet structure
 * ax = 0 (success) or non-zero (error) on return
 */
sector_read:
    mov ah, 0x42
    mov dl, 0x80
    int 0x13
    shr ah, 4  // return
    ret


/*
 * Load the VBR from the active parition of the MBR table
 * ds:di = seg:offset pointing to the first byte of the MBR in memory
 * ax = 0 (success) or non-zero (error) on return
 */
load_vbr:
    /* First, make sure we are looking at a valid MBR table by checking the signature */
    cmp word ptr ds:[di + 0x1fe], 0xaa55
    je find_active_partition 
    mov ax, 0xffff
    jmp load_vbr_done

/*
 * Entry 1: 0x1be
 * Entry 2: 0x1ce
 * Entry 3: 0x1de
 * Entry 4: 0x1ee
 */
find_active_partition:
    add di, 0x1be
    mov al, byte ptr [di]
    test al, al
    js active_part_found
    
    add di, 16
    mov al, byte ptr [di]
    test al, al
    js active_part_found

    add di, 16
    mov al, byte ptr [di]
    test al, al
    js active_part_found

    add di, 16
    mov al, byte ptr [di]
    test al, al
    js active_part_found
    mov ax, 0xffff
    jmp load_vbr_done

/* 
 * At this point we have found the active entry and di points to it 
 * Note that this also assumes si is set to the address of the data
 * address packet
 */
active_part_found:
    mov ax, word ptr [di + 8]
    mov bx, word ptr [di + 0x0a]
    lea si, da_packet
    mov byte ptr [si + 2], 0x01     // Data block count
    mov word ptr [si + 4], 0x7c00   // Offset
    mov word ptr [si + 6], 0x00     // Segment
    mov word ptr [si + 8], ax       // LBA (lower)
    mov word ptr [si + 10], bx      // LBA (upper)
    call sector_read    // Pass return through to caller of this function

load_vbr_done:
    ret
