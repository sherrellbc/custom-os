# Output config
#SYSROOT     ?= $(shell pwd)/../sysroot
SRCROOT         = ../../
SYSROOT         = .sysroot
BUILD           = .build
INCLUDEDIR      = /usr/include
HOST            = i686-elf
ARCH            = i386
ARCH_DIR        = arch/$(ARCH)
LIB_DIR         = ../../lib
BUILD_STR       = \e[1;34mBuilding $@\e[0m

# System resources
export CC=${HOST}-gcc --sysroot=$(BUILD) -isystem=$(INCLUDEDIR) -D__KERN__
export LD=${HOST}-ld
export AR=${HOST}-ar
export AS=${HOST}-as

# Note: 
#   --sysroot and -L are required for ld if invoking directly
#   GCC passes these parameters to ld if ld is invoked indirectly (through GCC)
CFLAGS      ?= -O0 -g #-masm=intel
CPPFLAGS    ?=
LDFLAGS     ?= #--sysroot=$(SYSROOT)
LIBS        ?=

CFLAGS      += -ffreestanding -Wall -Wextra -Iinclude -I$(ARCH_DIR)/include -Werror
LIBS        += -lc -lgcc -lmultiboot
LDFLAGS     += -nostdlib

KOBJS=  debug/printk/printk.o     \
        panic.o             \
        kernel.o            \
        irq/irq.o \

CLEAN_OBJS=$(KOBJS)

.PHONY: all clean install run
.SUFFIXES: .o .c .S

all: sysroot

include $(ARCH_DIR)/Makefile
include $(LIB_DIR)/Makefile

kernel.elf: install-build-kern-headers libs $(KOBJS) $(ARCH_DIR)/linker.ld
	@echo "\nBuilding $@"
	$(CC) -T $(ARCH_DIR)/linker.ld -o $@ $(LDFLAGS) $(KOBJS) $(CFLAGS) $(LIBS)
	objcopy --only-keep-debug $@ $@.sym

install: sysroot
sysroot: kernel.elf install-final-libs install-final-kernel install-final-bootloader sysroot-iso

install-build-kern-headers:
	mkdir -p $(BUILD)/$(INCLUDEDIR)
	cp -r $(ARCH_DIR)/include/* include/* $(BUILD)/$(INCLUDEDIR)	

install-final-kernel: kernel.elf
	mkdir -p $(SYSROOT)/$(INCLUDEDIR)
	cp -r $(BUILD)/$(INCLUDEDIR)/* $(SYSROOT)/$(INCLUDEDIR)
	mkdir -p $(SYSROOT)/boot
	cp kernel.elf $(SYSROOT)/boot/

GRUBCFG=$(SYSROOT)/boot/grub/grub.cfg
install-final-bootloader: 
	rm -f $(GRUBCFG)
	mkdir -p $(SYSROOT)/boot/grub
	echo "set timeout=0" >> $(GRUBCFG)
	echo "set default=0" >> $(GRUBCFG)
	echo "" >> $(GRUBCFG)
	echo "menuentry \"Custom OS\" {" >> $(GRUBCFG)
	echo "  multiboot /boot/kernel.elf arg0 arg1 arg2" >> $(GRUBCFG)
	echo "  boot" >> $(GRUBCFG)
	echo "}" >> $(GRUBCFG)

sysroot-iso: sysroot
	grub-mkrescue --output=mock.iso $(SYSROOT)

GDB_DEBUG=
ifneq ($(DEBUG),)
    GDB_DEBUG=-S -gdb tcp::10000
endif
runq: sysroot-iso
	qemu-system-i386 $(GDB_DEBUG) -boot d -cdrom mock.iso 
#	qemu-system-i386 $(GDB_DEBUG) -d int,cpu_reset -boot d -cdrom mock.iso 

runb: sysroot-iso
	bochs -qf ../../tools/bochs.config -rc ../../tools/bochs.cmds

.c.o:
	$(CC) -MD -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

.S.o:
	$(CC) -MD -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

clean:
	rm -rf $(BUILD)
	rm -rf $(SYSROOT)
	rm -f $(CLEAN_OBJS) $(CLEAN_OBJS:.o=.d) $(KOBJS:.o=.d)
	rm -f *.iso *.elf

# Output dependency info so we can track changes
-include $(OBJS:.o=.d)
