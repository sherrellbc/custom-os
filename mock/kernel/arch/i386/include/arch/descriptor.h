#ifndef _ASM_X86_SEGMENT_H
#define _ASM_X86_SEGMENT_H

#include <stdint.h>

// Each define here is for a specific flag in the descriptor.
// Refer to the intel documentation for a description of what each one does.
#define SEG_DESCTYPE(x)  ((x) << 0x04) // Descriptor type (0 for system, 1 for code/data)
#define SEG_PRES(x)      ((x) << 0x07) // Present
#define SEG_SAVL(x)      ((x) << 0x0C) // Available for system use
#define SEG_LONG(x)      ((x) << 0x0D) // Long mode
#define SEG_SIZE(x)      ((x) << 0x0E) // Size (0 for 16-bit, 1 for 32)
#define SEG_GRAN(x)      ((x) << 0x0F) // Granularity (0 for 1B - 1MB, 1 for 4KB - 4GB)
#define SEG_PRIV(x)     (((x) &  0x03) << 0x05)   // Set privilege level (0 - 3)


/*
 * See table 3-1 "Code and Data Segment Types" in IA32/64 SDM Vol. 3a
 */ 
#define SEG_DATA_RD        0x00     // Read-Only
#define SEG_DATA_RDA       0x01     // Read-Only, accessed
#define SEG_DATA_RDWR      0x02     // Read/Write
#define SEG_DATA_RDWRA     0x03     // Read/Write, accessed
#define SEG_DATA_RDEXPD    0x04     // Read-Only, expand-down
#define SEG_DATA_RDEXPDA   0x05     // Read-Only, expand-down, accessed
#define SEG_DATA_RDWREXPD  0x06     // Read/Write, expand-down
#define SEG_DATA_RDWREXPDA 0x07     // Read/Write, expand-down, accessed
#define SEG_CODE_EX        0x08     // Execute-Only
#define SEG_CODE_EXA       0x09     // Execute-Only, accessed
#define SEG_CODE_EXRD      0x0A     // Execute/Read
#define SEG_CODE_EXRDA     0x0B     // Execute/Read, accessed
#define SEG_CODE_EXC       0x0C     // Execute-Only, conforming
#define SEG_CODE_EXCA      0x0D     // Execute-Only, conforming, accessed
#define SEG_CODE_EXRDC     0x0E     // Execute/Read, conforming
#define SEG_CODE_EXRDCA    0x0F     // Execute/Read, conforming, accessed
 
#define GDT_CODE_PL0 SEG_DESCTYPE(1) | SEG_PRES(1) | SEG_SAVL(0) | \
                     SEG_LONG(0)     | SEG_SIZE(1) | SEG_GRAN(1) | \
                     SEG_PRIV(0)     | SEG_CODE_EXRD
 
#define GDT_DATA_PL0 SEG_DESCTYPE(1) | SEG_PRES(1) | SEG_SAVL(0) | \
                     SEG_LONG(0)     | SEG_SIZE(1) | SEG_GRAN(1) | \
                     SEG_PRIV(0)     | SEG_DATA_RDWR
 
#define GDT_CODE_PL3 SEG_DESCTYPE(1) | SEG_PRES(1) | SEG_SAVL(0) | \
                     SEG_LONG(0)     | SEG_SIZE(1) | SEG_GRAN(1) | \
                     SEG_PRIV(3)     | SEG_CODE_EXRD
 
#define GDT_DATA_PL3 SEG_DESCTYPE(1) | SEG_PRES(1) | SEG_SAVL(0) | \
                     SEG_LONG(0)     | SEG_SIZE(1) | SEG_GRAN(1) | \
                     SEG_PRIV(3)     | SEG_DATA_RDWR


/*
 * The convoluted definition a single segment descriptor is as follows:
 *
 * [Bits]       [Use]                       [Length]    [Description]
 * 00 - 15      Segment Limit   (0  - 15)   16          Selected segment length
 * 16 - 31      Base Address    (0  - 15)   16          Selected segment base address
 * 32 - 39      Base Address    (16 - 23)   8
 * 40 - 43      Type            (0  -  3)   4           LDT, TSS, Gate, Code, Data
 * 44           S               (0      )   1           0=System, 1=User
 * 45 - 47      DPL             (0  -  1)   2           "Ring level", 0=most priv, 3=least priv
 * 48           P               (0      )   1           Indicates whether the segment is loaded into memory
 * 49 - 51      Segment Limit   (16 - 19)   4
 * 52           AVL             (0      )   1           Available to Software (Processor does not set/clear this field)
 * 53           Reserved                    1
 * 54           DB              (0      )   1           Default Operand Size (Code/Data segments only); 1=32 bit, 0=16 bit
 * 55           G               (0      )   1           Granularity (scaling) of limit field; 0=no scaling, 1=4KiB (i.e. 4096 byte blocks)
 * 56 - 63      Base Address    (24 - 31)   8
 */
struct segment_desc{
    union{
        struct {
            uint64_t entry;
        };
        
        struct {
            uint32_t dword0;
            uint32_t dword1;
        };

        struct{
            /* Legacy descriptor */
            uint16_t limit; 
            uint16_t base;

            /* Extensions */
            uint16_t base1  :8, type :4, s   :1, dpl :2, p :1;
            uint16_t limit1 :4, avl  :1, res :1, db  :1, g :1, base2 :8;
        };
    };
} __attribute__((packed));


struct gate_desc {
    union {
        uint64_t entry;
        
        struct {
            uint32_t dword0;
            uint32_t dword1;
        };
        
        struct {
            uint16_t handler_addr0;
            uint16_t selector;
            uint8_t mbz;
            uint8_t type_attr;// :4, s :1, dpl :2, p :1;
            uint16_t handler_addr1;    
        }; 
    };
} __attribute__((packed));


/*
 * flags is defined as g (1), db (1), res (1), a (1), zeros (4), p (1), dpl (2), s (1), type (3)
 */
#define SEGMENT_FLAGS(g,db,avl,p,dpl,s,type)  (((g) << 15) | ((db) << 14) | ((avl) << 12) | ((p) << 7) | ((dpl) << 5) | ((s) << 4) | (type))
#define GDT_DESCRIPTOR_ENTRY(_base, _limit, _flags) \
    (struct segment_desc) { \
        .dword0 = (((_base) & 0xffff) << 16) | ((_limit) & 0xffff), \
        .dword1 = ((_base) & 0xff000000) | (((_flags) & 0xd0ff) << 8) | ((_limit) & 0xf0000) | ((_base) & 0xff0000 >> 16) \
    } \


#define SELECTOR(rpl, ti, index)    ((index << 3) | (ti << 2) | (rpl & 0x3))

#define LDT_DESCRIPTOR_ENTRY(_handler, _selector, _type) \
    (struct gate_desc) { \
        .handler_addr0 = (uint16_t) (((uint32_t)_handler) & 0xffff), \
        .selector = (_selector), \
        .mbz = 0, \
        .type_attr = _type, \
        .handler_addr1 = (uint16_t) ((((uint32_t)_handler) & 0xffff0000) >> 16), \
    } \


/*
 * Specifies the location of a current descriptor table (.e.g. GDT, LDT, etc)
 * Note: The address of a structure of this type will be used with the corresponding {l,s}{gdt,idt} instruction
 */
struct desc_table_ptr{
    uint16_t len;           /* Bits 0  - 15 */
    uint32_t ptr;           /* Bits 16 - 31 */
} __attribute__((packed));



static inline void load_gdt(struct desc_table_ptr *src)
{
    asm volatile("lgdt %0": :"m" (*src));
}


static inline void store_gdt(struct desc_table_ptr *dst)
{
    asm volatile("sgdt %0": :"m" (*dst));
}


static inline void load_idt(struct desc_table_ptr *src)
{
    asm volatile("lidt %0": :"m" (*src));
}


static inline void store_idt(struct desc_table_ptr *dst)
{
    asm volatile("sidt %0": :"m" (*dst));
}


/* 
 * Install a default default Global Descriptor Table (GDT) for kernel and userspace
 */
void gdt_setup(void);


/*
 * Set a specified entry in the currently installed GDT and optionally force a GDT reload
 *
 * @param slot      The desired slot to retrieve
 * @param entry     The entry to be installed (only copied, no references are maintained)
 * @param reload    A switch to force the GDT to be reloaded/cached by the CPU
 * @return          Non-zero if failure
 */ 
int gdt_set_slot(int slot, struct segment_desc *entry, int reload);


/*
 * Get a specified entry from the currently installed GDT
 *
 * @param slot      The desired slot to retrieve
 * @param dst       A caller-allocated location to store the retrieved entry
 * @return          Non-zero if failure
 */ 
int gdt_get_slot(int slot, struct segment_desc *dst);


/*
 * Install a default Interrupt Descriptor Table (IDT) with default handler entries
 */
void idt_setup(void);


/*
 * Set a specified entry from the currently installed IDT
 *
 * @param slot      The desired slot to retrieve
 * @param entry     The entry to be installed (only copied, no references are maintained)
 * @return          Non-zero if failure
 */ 
int idt_set_slot(int slot, struct gate_desc *entry);


/*
 * Get a specified entry from the currently installed IDT
 *
 * @param slot      The desired slot to retrieve
 * @param dst       A caller-allocated location to store the retrieved entry
 * @return          Non-zero if failure
 */ 
int idt_get_slot(int slot, struct gate_desc *dst);


#endif /* _ASM_X86_SEGMENT_H */
