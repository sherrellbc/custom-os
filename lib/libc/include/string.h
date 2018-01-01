#ifndef _STRING_H_
#define _STRING_H_

#include <stddef.h>


int strncmp(const char *s1, const char *s2, size_t n);
char *strncpy(char * restrict dst, char * restrict src, size_t len);
int memcmp(const void*, const void*, size_t);
void* memcpy(void* __restrict, const void* __restrict, size_t);
void* memmove(void*, const void*, size_t);
void* memset(void*, int, size_t);
size_t strlen(const char*);
char *strrev(char *str);


#endif /* _STRING_H_ */
