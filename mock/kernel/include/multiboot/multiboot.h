#ifndef _MULTIBOOT_H
#define _MULTIBOOT_H

#include <stdint.h>
#include "multiboot_legacy.h"

/*
 * Initializes the multiboot module
 * @param mbi   : Multiboot structure passed to us from the bootloader
 * @param magic : The 32-bit magic passed to us from the bootloader
 * @return      : -1 on failure
 */
int mb_init(struct multiboot_info *mbi, uint32_t magic);


/*
 * Checks the validity of the multiboot structure
 * @param mbi   : Multiboot structure to check
 * @return      : -1 on failure
 */
int mb_check_valid(struct multiboot_info *mbi);


#endif /* _MULTIBOOT_H */
