target remote localhost:10000
set disassembly-flavor intel

# Default breakpoints
#break kernel_main
#break kernel_loader
break int_handler
break kernel_main

# Layout schema
#layout asm
#layout regs 
layout src

#display/w $esp
#display/w $eax

c
display/"\nBuf=%s",buf
display/"neg=%d",neg_flag
display/"digit=%d",digit
display/"num=%x",num
