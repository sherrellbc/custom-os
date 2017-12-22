/* 
 * Define the entry point 
 * - ENTRY(symbol) can be used here or inside the SECTIONS block
 * - start = symbol; may also be used
 * - defaults to first byte of .text section if left undefined
 */
ENTRY(kernel_loader)

/* 
 * Specify the default output architecture and file format
 * - Often, these parameters are defined implicitly at link-time by consulting the local BFD (Binary File Descriptor) 
 * library configuration. Experimentation yielded the default output was x86_64 machine code wrapped in a ELF64 header 
 * (regardless of machine code type). As such, it must be defined explicitly if building for a different target/format
 * than your local configuration defaults to. 
 *
 * REF: https://ftp.gnu.org/old-gnu/Manuals/ld-2.9.1/html_mono/ld.html#SEC3
 */
OUTPUT_ARCH(i386:i386)
OUTPUT_FORMAT(elf32-i386)

STACK_SIZE = 0x8000;

SECTIONS
{
    . = 0x00100000;
    .text :                         /* multiboot structure must be contained within first 8192 bytes and longword (32) bit aligned */
    {   
        __multiboot_header = .;
        *(.multiboot)

        __text_start = .;
        . = ALIGN(16);
        *(.text*)                   /* All our code */
        *(.rodata)                  /* Any read-only data */
        __text_end = .; 
     }

    .data ALIGN(4):
    {
        __data_start = .;
        *(.data)                    /* Initialized data */
        __data_end = .;
    }

    .bss ALIGN(4) :
    {
        __bss_start = .; 
        *(.bss)                     /* All uninitialized data */
        __bss_end = .;
    }

     . = ALIGN(4);
    __stacktop = . + STACK_SIZE;
    __stacklimit = __stacktop - STACK_SIZE;
    PROVIDE(__stack = __stacktop);
}
