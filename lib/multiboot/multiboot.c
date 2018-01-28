#include <stdio.h>
//#include <kernel/.h>
#include <multiboot/multiboot.h>

#define CHECK_FLAG(flags,bit)   ((flags) & (1 << (bit)))

#ifdef MB_DEBUG
    #include <mock.h> 
    #define mb_print(fmt, ...)  printk(fmt, ##__VA_ARGS__)
#else
    #define mb_print(fmt, ...)
#endif

/*
 * Checks the validity of the multiboot structure
 * @param mbi   : Multiboot structure to check
 * @return      : -1 on failure
 */
int mb_check_valid(struct multiboot_info *mbi)
{
    /* Print out the flags. */
    mb_print("flags = 0x%x\n", (unsigned) mbi->flags);

    /* Are mem_* valid? */
    if (CHECK_FLAG (mbi->flags, 0)){
        mb_print("mem_lower = %uKB, mem_upper = %uKB\n",
                (unsigned) mbi->mem_lower, (unsigned) mbi->mem_upper);
    }

    /* Is boot_device valid? */
    if (CHECK_FLAG (mbi->flags, 1)){
        mb_print("boot_device = 0x%x\n", (unsigned) mbi->boot_device);
    }

    /* Is the command line passed? */
    if (CHECK_FLAG (mbi->flags, 2)){
        mb_print("cmdline = %s\n", (char *) mbi->cmdline);
    }

    /* Are mods_* valid? */
    if (CHECK_FLAG (mbi->flags, 3)) {
        multiboot_module_t *mod;
        int i;

        mb_print("mods_count = %d, mods_addr = 0x%x\n",
                (int) mbi->mods_count, (int) mbi->mods_addr);
        for (i = 0, mod = (multiboot_module_t *) mbi->mods_addr;
                i < (int) mbi->mods_count;
                i++, mod++)
            mb_print(" mod_start = 0x%x, mod_end = 0x%x, cmdline = %s\n",
                    (unsigned) mod->mod_start,
                    (unsigned) mod->mod_end,
                    (char *) mod->cmdline);
    }

    /* Bits 4 and 5 are mutually exclusive! */
    if (CHECK_FLAG (mbi->flags, 4) && CHECK_FLAG (mbi->flags, 5)) {
        mb_print("Both bits 4 and 5 are set.\n");
        return -1;
    }

    /* Is the symbol table of a.out valid? */
    if (CHECK_FLAG (mbi->flags, 4)) {
        multiboot_aout_symbol_table_t *multiboot_aout_sym = &(mbi->u.aout_sym);

        mb_print("multiboot_aout_symbol_table: tabsize = 0x%0x, "
                "strsize = 0x%x, addr = 0x%x\n",
                (unsigned) multiboot_aout_sym->tabsize,
                (unsigned) multiboot_aout_sym->strsize,
                (unsigned) multiboot_aout_sym->addr);
    }

    /* Is the section header table of ELF valid? */
    if (CHECK_FLAG (mbi->flags, 5)) {
        multiboot_elf_section_header_table_t *multiboot_elf_sec = &(mbi->u.elf_sec);

        mb_print("multiboot_elf_sec: num = %u, size = 0x%x,"
                " addr = 0x%x, shndx = 0x%x\n",
                (unsigned) multiboot_elf_sec->num, (unsigned) multiboot_elf_sec->size,
                (unsigned) multiboot_elf_sec->addr, (unsigned) multiboot_elf_sec->shndx);
    }

    /* Are mmap_* valid? */
    if (CHECK_FLAG (mbi->flags, 6)) {
        multiboot_memory_map_t *mmap;

        mb_print("mmap_addr = 0x%x, mmap_length = 0x%x\n",
                (unsigned) mbi->mmap_addr, (unsigned) mbi->mmap_length);

        mb_print("Memory map:\n");
        mmap = (multiboot_memory_map_t *) mbi->mmap_addr;
        for (; (uintptr_t) mmap < mbi->mmap_addr + mbi->mmap_length; 
                mmap = (multiboot_memory_map_t *) ((unsigned long) mmap + sizeof(multiboot_memory_map_t))){

            mb_print("\tbase = 0x%x%x, len = 0x%x%x, type = %s\n",
                (uint32_t) (mmap->addr >> 32),
                (uint32_t)  mmap->addr,
                (uint32_t) (mmap->len >> 32),
                (uint32_t)  mmap->len,
                ((uint32_t) mmap->type) == 1 ? "RAM" : "Reserved");
        }
    }

    return 0;
}


int mb_init(struct multiboot_info *mbi, uint32_t magic)
{
    /* Sanity check before proceeding */
    if(MULTIBOOT_BOOTLOADER_MAGIC != magic)
        return -1;

    return mb_check_valid(mbi);
}
