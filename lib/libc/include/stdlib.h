#ifndef _STDLIB_H
#define _STDLIB_H


/*
 * Effectively hang the system i
 */ 
//__attribute__((__noreturn__))
void abort(void);


/*
 * Convert an integer of a given base to its ASCII equivalent. Of course, the current implementation 
 * is not reentrant. 
 * @param num   : The number to convert
 * @param base  : The base of the passed intger
 * @return      : A pointer to a local static buffer containing the converted result
 */
char *itoa(int num, int base); 


/*
 * Convert an integer of a given base to its ASCII equivalent. Of course, the current implementation 
 * is not reentrant. 
 * @param num   : The number to convert
 * @param base  : The base of the passed intger
 * @return      : A pointer to a local static buffer containing the converted result
 */
char *ltoa(long long num, int base); 


/*
 * The reetrant equivalent of itoa
 * @param num   : The number to convert
 * @param str   : Buffer to copy the results
 * @param len   : The length of the buffer passed
 * @param base  : The base of the passed intger
 * @return      : A pointer to the buffer containing the converted result
 */
char *itoa_r(int num, char *str, int len, int base);
 
#endif
