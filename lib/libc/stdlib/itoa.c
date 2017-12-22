#include <string.h>
#define NUM_DIGITS 32 


/*
 * Convert an integer of a given base to its ASCII equivalent. This implementation is not reentrant 
 * is not reentrant. 
 * @param num   : The number to convert
 * @param base  : The base of the passed intger
 * @return      : A pointer to a local static buffer containing the converted result
 */
char *itoa(int num, int base) 
{
    static char buf[NUM_DIGITS] = {0};
    unsigned int digit;
    unsigned int number = (unsigned int) num;
    int neg_flag = 0;
    char *dst = buf + sizeof(buf)-1;  /* Start at the end */

    memset(buf,'Z',32);
    buf[31] = '\0';

    /* Special case for base 10 if the passed integer is negative */
    if( (num < 0) && (10 == base) ){
        neg_flag = 1;
        num *= -1;
    }
    
    /* While we haven't converted all digits and still have room for nul-term */
    do{
        digit = number % base;
        if (digit < 0xA)
            *--dst = '0' + digit;
        else
            *--dst = 'A' + digit - 0xA;
        number /= base;
    }while(number != 0);

    /* We worked backwards, so the negative case should be the last thing we check for */
    if(1 == neg_flag)
        *--dst = '-';

    return dst;
}


/*
 * Convert a long long integer of a given base to its ASCII equivalent. This implementation is not reentrant 
 * @param num   : The number to convert
 * @param base  : The base of the passed intger
 * @return      : A pointer to a local static buffer containing the converted result
 */
char *ltoa(long long num, int base) 
{
    static char buf[NUM_DIGITS] = {0};
    unsigned int digit;
    unsigned long long number = (unsigned long long) num;
    int neg_flag = 0;
    char *dst = buf + sizeof(buf)-1;  /* Start at the end */

    /* Special case for base 10 if the passed integer is negative */
    if( (num < 0) && (10 == base) ){
        neg_flag = 1;
        num *= -1;
    }
    
    /* While we haven't converted all digits and still have room for nul-term */
    do{
        digit = number % base;
        if (digit < 0xA)
            *--dst = '0' + digit;
        else
            *--dst = 'A' + digit - 0xA;
        number /= base;
    }while(number != 0);

    /* We worked backwards, so the negative case should be the last thing we check for */
    if(1 == neg_flag)
        *--dst = '-';

    return dst;
}


/*
 * The reetrant equivalent of itoa
 * @param num   : The number to convert
 * @param str   : Buffer to copy the results
 * @param len   : The length of the buffer passed
 * @param base  : The base of the passed intger
 * @return      : A pointer to the buffer containing the converted result
 */
char *itoa_r(int num, char* str, int len, int base)
{
    int digit;
    int i  = 0;

    /* Sanity check */
    if( (len == 0) || (NULL == str) ) return NULL;

    /* Special case for base 10 if the passed integer is negative */
    if( (num < 0) && (base == 10) )
        str[i++] = '-';

    /* While we haven't converted all digits and still have room for nul-term */
    do{
        digit = num % base;
        if (digit < 0xA)
            str[i++] = '0' + digit;
        else
            str[i++] = 'A' + digit - 0xA;
        
        num /= base;
    }while( (num > 0) && (i < (len - 1)) );

    /* Not enough room in buffer */
    if( (i == (len - 1)) && (num > 0) )
        return NULL;

    str[i] = '\0';
    strrev(str);
    return str;
}
