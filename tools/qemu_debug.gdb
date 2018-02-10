target remote localhost:10000
set disassembly-flavor intel

#file ./kernel.elf

# Default breakpoints
#break default_handler
#break time_systick_handler

# Layout schema
layout asm
layout regs 
#layout src

#display/w $esp
#display/w $eax
set architecture i8086
c
