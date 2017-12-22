#include <stdint.h>


/*
 * Root System Descriptor Pointer
 */
struct RSDPDescriptor {
    char Signature[8];          /* Should be "RSD PTR "; note the trailing space */
    uint8_t Checksum;
    char OEMID[6];
    uint8_t Revision;
    uint32_t RsdtAddress;       /* Address of the Root System Descriptor Table */
} __attribute__ ((packed));



/*
 * Search the main BIOS memory for the RSDP pointer
 * Per the spec, it is guarenteed to be 16-byte aligned
 *
 * @return  The address of the RSDP structure, or NULL if not found
 */
struct RSDPDescriptor* find_rsdp(void);
