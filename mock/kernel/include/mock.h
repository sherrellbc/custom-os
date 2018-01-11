#ifndef _MOCK_H
#define _MOCK_H


#include <kernel/printk.h>
#include <platform.h>

//XXX: This should be in a "debug" area of the headers, and defined only if a debug build
#define assertk(expr) ({                                                    \
            if(0 == (expr)){                                                \
                printk("Assert failed: %s:%d\n", __FUNCTION__, __LINE__);   \
                while(1);                                                   \
            }                                                               \
        })                                                                  \


#endif /* _MOCK_H */
