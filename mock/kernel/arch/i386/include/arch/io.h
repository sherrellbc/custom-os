#ifndef _ASM_X86_IO_H
#define _ASM_X86_IO_H



#define CREATE_IO_FUNC_TYPE(bwl, bw, type)                  \
static inline void out##bwl(unsigned type value, int port)  \
{                                                           \
    asm volatile("out" #bwl " %" #bw "0, %w1"               \
             : : "a"(value), "Nd"(port));                   \
}                                                           \
                                                            \
static inline unsigned type in##bwl(int port)               \
{                                                           \
    unsigned type value;                                    \
    asm volatile("in" #bwl " %w1, %" #bw "0"                \
             : "=a"(value) : "Nd"(port));                   \
    return value;                                           \
}                                                           \


CREATE_IO_FUNC_TYPE(b, b, char)
CREATE_IO_FUNC_TYPE(w, w, short)
CREATE_IO_FUNC_TYPE(l, , int)


#endif /* _ASM_X86_IO_H */
