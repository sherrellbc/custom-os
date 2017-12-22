# Architecture-specific build flags
KERNEL_ARCH_CFLAGS=
KERNEL_ARCH_CPPFLAGS=
KERNEL_ARCH_LDFLAGS=
KERNEL_ARCH_LIBS=

CFLAGS      += $(KERNEL_ARCH_CFLAGS)
CPPFLAGS    += $(KERNEL_ARCH_CPPFLAGS)
LDFLAGS     += $(KERNEL_ARCH_LDFLAGS)

KERNEL_ARCH_OBJS=\
    $(ARCH_DIR)/boot/loader.o \
    $(ARCH_DIR)/boot/boot.o \
    $(ARCH_DIR)/descriptor.o \
    $(ARCH_DIR)/irq_handlers.o \
    $(ARCH_DIR)/irq.o \
    $(ARCH_DIR)/pic8259.o \
    $(ARCH_DIR)/tty.o \

KOBJS+=$(KERNEL_ARCH_OBJS) 
CLEAN_OBJS+=$(KOBJS)
