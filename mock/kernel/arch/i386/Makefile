# Architecture-specific build flags
KERNEL_ARCH_CFLAGS=
KERNEL_ARCH_CPPFLAGS=
KERNEL_ARCH_LDFLAGS=
KERNEL_ARCH_LIBS=

CFLAGS      += $(KERNEL_ARCH_CFLAGS)
CPPFLAGS    += $(KERNEL_ARCH_CPPFLAGS)
LDFLAGS     += $(KERNEL_ARCH_LDFLAGS)

KERNEL_EARLY_PLATFORM_INIT=\
    $(ARCH_DIR)/boot/loader.o \
    $(ARCH_DIR)/boot/boot.o \
    $(ARCH_DIR)/descriptor.o \

KERNEL_IRQ_OBJS=\
    $(ARCH_DIR)/irq/irq_core/default_handler.o \
    $(ARCH_DIR)/irq/irq_core/irq.o \
    $(ARCH_DIR)/irq/irq_core/pic8259.o \
    $(ARCH_DIR)/irq/irq_core/apic.o \
    $(ARCH_DIR)/irq/keyboard/keyboard.o \
    $(ARCH_DIR)/irq/keyboard/keyboard_asm.o \
    $(ARCH_DIR)/irq/time/time.o \
    $(ARCH_DIR)/irq/time/time_asm.o \

KERNEL_MISC_OBJS=\
    $(ARCH_DIR)/tty.o \

KERNEL_ARCH_OBJS=$(KERNEL_EARLY_PLATFORM_INIT) $(KERNEL_IRQ_OBJS) $(KERNEL_MISC_OBJS)
KOBJS+=$(KERNEL_ARCH_OBJS) 
CLEAN_OBJS+=$(KOBJS)
