#include <string.h>
#include <acpi/rsdp.h>


/*
 * Search the main BIOS memory for the RSDP pointer
 * Per the spec, it is guarenteed to be 16-byte aligned
 *
 * @return  The address of the RSDP structure, or NULL if not found
 */
struct RSDPDescriptor* find_rsdp(void){
    uint32_t base_addr = 0xB0000;

    for(; base_addr<=0xFFFFF; base_addr+=16){
        if(0 == strncmp("RSD PTR ", (char*)base_addr, sizeof("RSD PTR ")-1))
            return (struct RSDPDescriptor *) base_addr; 
    }

    return NULL;
}
