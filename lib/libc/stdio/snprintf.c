#include <stdio.h>
#include <stdlib.h>


int snprintf(char *buffer, size_t size, const char *fmt, ...)
{
    int ret; 

    va_list arg;
    va_start(arg, fmt);
    ret = vsnprintf(fmt, size, buffer, arg);
    va_end(arg);

    return ret; 
}


int vsnprintf(const char* fmt, size_t size, char* buffer, va_list arg)
{
    int int_param, index;
    long long ll_param;
    size_t buf_index;
    char *conv_result;

    /* Sanity check */
    if( (0 == size) || (NULL == buffer) ) return -1;

    /* Simple case */
    if(1 == size){
        buffer[0] = '\0';
        return 1;
    }

    index = 0;
    buf_index = 0;
    while(fmt[index] != '\0') {
    
        /* Check for size-related exit case */
        if(buf_index == size-1)
            goto exit;

        switch(fmt[index]) {
        case '%':
            switch(fmt[index+1]) {
            case 'd':
            case 'u':
                int_param = va_arg(arg, int);
                conv_result = itoa(int_param, 10);

                for(; *conv_result != '\0'; conv_result++){
                    buffer[buf_index++] = *conv_result;
                    if(buf_index == size-1)
                        goto exit;
                }
                
                index+=2;
                break;

            //FIXME: 'l' Does not imply long long, nor base 10. This whole parser needs to be rewritten
            case 'l':
                ll_param = va_arg(arg, long long);
                conv_result = ltoa(ll_param, 10);

                for(; *conv_result != '\0'; conv_result++){
                    buffer[buf_index++] = *conv_result;
                    if(buf_index == size-1)
                        goto exit;
                }

                index+=2;
                break;

            case 'x':
                int_param = va_arg(arg, int);
                conv_result = itoa(int_param, 16);

                for(; *conv_result != '\0'; conv_result++){
                    buffer[buf_index++] = *conv_result;
                    if(buf_index == size-1)
                        goto exit;
                }

                index+=2;
                break;

            case 'c':
                buffer[buf_index++] = va_arg(arg, int);
                index+=2;
                break;

            case 's':
                conv_result = va_arg(arg, char*);

                for(; *conv_result != '\0'; conv_result++){
                    buffer[buf_index++] = *conv_result;
                    if(buf_index == size-1)
                        goto exit;
                }

                index+=2;
                break;

            }
        break;
        default:
            buffer[buf_index++] = fmt[index++];
            break;
        }
    }

exit:
    buffer[buf_index++] = '\0';
    return buf_index;
}
