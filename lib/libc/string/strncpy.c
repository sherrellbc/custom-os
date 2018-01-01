#include <string.h>


char *strncpy(char * restrict dst, char * restrict src, size_t len)
{
    char *dst_start = dst;

    while( (*src != '\0') && ((size_t)(dst - dst_start) < len)){
        *(dst)++ = *(src++);
    }

    return dst;
}
