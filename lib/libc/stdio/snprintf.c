#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>


/* 
 * TODO
 * - Implement proper formatting using the already collected flag and width information
 * - Implement proper boundary checking (implement macro for writing to buffer that jumps to out label when full
 * - Implement proper printing of long, long long, and pointer types
 */


/*
 * State definitions for snprintf as it parses a format string
 */
enum state {
    STATE_CHAR,
    STATE_PERCENT_SIGN,
    STATE_FLAGS,
    STATE_WIDTH,
    STATE_PRECISION,
    STATE_LENGTH,
    STATE_SPECIFIER
};

/* Length definitions */
enum length_modes {
    LENGTH_H,
    LENGTH_HH,
    LENGTH_L,
    LENGTH_LL,
    LENGTH_Z,
    LENGTH_UNKNOWN,
    LENGTH_NONE
};

enum flag_modes {
    FLAG_LEFT_JUSTIFY = 0x01,
    FLAG_HASH =         0x02,
    FLAG_ZERO =         0x04,
    FLAG_SPACE =        0x08,
    FLAG_PLUS =         0x10
};

struct out_format {
    int flags;
    int width;
    int length;
    int precision;
};


/*
 * Note that some of the special cases (e.g. tab) must be implemented 
 * explicitly if printing to the VGA console, which is not a _real_
 * console program that interprets control keys 
 */
int vsnprintf(char *buf, size_t len, const char *format, va_list va)
{
    struct out_format out = {0};
    int state = STATE_CHAR;
    int f_index = 0;
    int b_index = 0;
    int index_increment = 0;

    while('\0' != format[f_index]){

        switch(state){
            case STATE_CHAR:
                /* Check character special cases first */
                switch(format[f_index]){
                    case '%':
                        state = STATE_PERCENT_SIGN;
                        break;
                    default:
                        buf[b_index++] = format[f_index];
                        break;
                }
                
                index_increment = 1;
                break;

            case STATE_PERCENT_SIGN:
                if('%' == format[f_index]){
                    buf[b_index++] = format[f_index];
                    state = STATE_CHAR;
                    index_increment = 1;    /* For safety, but STATE_CHAR has already set this */
                    break;
                }

                state = STATE_FLAGS;
                index_increment = 0;
                break;

            case STATE_FLAGS:
                switch(format[f_index]){
                    case '-':
                        out.flags |= FLAG_LEFT_JUSTIFY;
                        index_increment = 1; 
                        break;
                    case '+':
                        buf[b_index++] = '+';
                        out.flags |= FLAG_PLUS;
                        index_increment = 1; 
                        break;
                    case '#':
                        out.flags |= FLAG_HASH;
                        index_increment = 1; 
                        break;
                    case '0': 
                        out.flags |= FLAG_ZERO;
                        index_increment = 1; 
                        break;
                    case ' ':
                        out.flags |= FLAG_SPACE;
                        index_increment = 1; 
                        break;
                    default:
                        index_increment = 0; 
                        break;
                }

                state = STATE_WIDTH;
                break;

            case STATE_WIDTH:
                if('*' == format[f_index]){
                    out.width = va_arg(va, int);
                    index_increment = 1;
                }else{ 
                    //TODO: implement width
                    index_increment = 0;
                }

                state = STATE_PRECISION;
                break;

            case STATE_PRECISION:
                if('*' == format[f_index]){
                    out.precision = va_arg(va, int);
                    index_increment = 1;
                }else{ 
                    //TODO: implement precision
                    index_increment = 0;
                }
                state = STATE_LENGTH;
                break;

            case STATE_LENGTH:
                switch(format[f_index]){
                    case 'h':
                        if('h' == format[f_index + 1]){
                            out.length = LENGTH_HH;
                            index_increment = 2;
                        }else{
                            out.length = LENGTH_H;
                            index_increment = 1;
                        }
                        break;
                    case 'l':
                        if('l' == format[f_index + 1]){
                            out.length = LENGTH_LL;
                            index_increment = 2;
                        }else{
                            out.length = LENGTH_L;
                            index_increment = 1; 
                        }
                        break;
                    case 'z':
                        out.length = LENGTH_Z;
                        index_increment = 1; 
                        break;
                    /* Some unimplemented length modifiers */
                    case 'j':
                    case 't':
                    case 'L':
                        out.length = LENGTH_UNKNOWN;
                        index_increment = 1; 
                        break; 
                    default:
                        out.length = LENGTH_NONE;
                        index_increment = 0; 
                        break;
                }

                state = STATE_SPECIFIER;
                break;

             /* Note: short/char are promoted to int when passed to variadic functions */
            case STATE_SPECIFIER:
            {
                char *str;
                int base = 10;

                switch(format[f_index]){
                    case 'x':
                    case 'X':
                    case 'd':
                    case 'i':
                    case 'u':
                    {
                        /* Certain cases that land here will change the base of the interpretation */
                        if( 'x' == format[f_index] || 'X' == format[f_index] ){
                            base = 16;
                        }
            
                        switch(out.length){
                            case LENGTH_NONE:
                            case LENGTH_H:
                            case LENGTH_HH:
                                str = itoa(va_arg(va, int), base);
                                break;
                            case LENGTH_L:
                                str = itoa(va_arg(va, long), base);
                                break;
                            case LENGTH_LL:
                                str = itoa(va_arg(va, long long), base);
                                break;
                            case LENGTH_Z:
                                str = itoa(va_arg(va, size_t), base);
                                break;
                            default:
                                buf[b_index++] = va_arg(va, int);       /* Treat the unknown case like characters */
                                break;
                        }
                        memcpy(&buf[b_index], str, strlen(str));
                        b_index += strlen(str);
                    }
                        break;
                    case 'c':
                        buf[b_index++] = va_arg(va, int);
                        break;
                    case 's':
                    {
                        str = va_arg(va, char *);
                        strncpy(&buf[b_index], str, len-b_index);
                        b_index += strlen(str);
                    }
                        break;
                    case 'p':
                    {
                        
                        str = itoa((int)va_arg(va, void*), 16);
                        memcpy(&buf[b_index], str, strlen(str));
                        b_index += strlen(str);
                    }
                        break;
                    default:
                        buf[b_index++] = '?';
                        break;
                }        
                index_increment = 1; 
                state = STATE_CHAR;
            }
                break;

            default:
                break;
        }

        f_index += index_increment;
    }

//TODO: create a macro for adding elements to the buffer that jumps here if it does not fit
//add_newline_exit:
    buf[b_index++] = '\0';
    return b_index;
}


int snprintf(char *buf, size_t size, const char *format, ...)
{
    int ret; 

    va_list arg;
    va_start(arg, format);
    ret = vsnprintf(buf, size, format, arg);
    va_end(arg);

    return ret; 
}
