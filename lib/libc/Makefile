LIBC_LOCALDIR:=$(dir $(lastword $(MAKEFILE_LIST)))
LIBC_BUILD_INSTALL_DIR=/usr/lib
LIBC_BUILD_INCLUDE_INSTALL_DIR=/usr/include

# Define build objects, include architecture-specific ones
LIBC_OBJS =\
    $(LIBC_LOCALDIR)/string/strncmp.o \
    $(LIBC_LOCALDIR)/string/strlen.o \
    $(LIBC_LOCALDIR)/string/memcmp.o \
    $(LIBC_LOCALDIR)/string/memmove.o \
    $(LIBC_LOCALDIR)/string/memset.o \
    $(LIBC_LOCALDIR)/string/strrev.o \
    $(LIBC_LOCALDIR)/stdlib/abort.o  \
    $(LIBC_LOCALDIR)/stdlib/itoa.o  \
    $(LIBC_LOCALDIR)/stdio/putchar.o \
    $(LIBC_LOCALDIR)/stdio/snprintf.o \

CLEAN_OBJS += $(LIBC_OBJS)

install-libc-build-headers: 
	mkdir -p $(BUILD)/$(LIBC_BUILD_INCLUDE_INSTALL_DIR)
	cp -r $(LIBC_LOCALDIR)/include/* $(BUILD)/$(LIBC_BUILD_INCLUDE_INSTALL_DIR)

install-final-libc: install-libc-final-headers
	#install userspace libc

install-libc-final-headers: install-libc-build-headers
	mkdir -p $(SYSROOT)/$(LIBC_BUILD_INCLUDE_INSTALL_DIR)
	cp -r $(BUILD)/$(LIBC_BUILD_INCLUDE_INSTALL_DIR)/* $(SYSROOT)/$(LIBC_BUILD_INCLUDE_INSTALL_DIR)/

libc: install-libc-build-headers $(LIBC_OBJS)
	@echo "\nBuilding $@"
	$(AR) rcs $(LIBC_LOCALDIR)/$@.a $(LIBC_OBJS)
	mkdir -p $(BUILD)/$(LIBC_BUILD_INSTALL_DIR)
	cp $(LIBC_LOCALDIR)/$@.a $(BUILD)/$(LIBC_BUILD_INSTALL_DIR)
