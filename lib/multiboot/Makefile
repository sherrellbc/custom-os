MULTIBOOT_LOCALDIR:=$(dir $(lastword $(MAKEFILE_LIST)))
MULTIBOOT_BUILD_INSTALL_DIR=/usr/lib
MULTIBOOT_BUILD_INCLUDE_INSTALL_DIR=/usr/include

# Define build objects, include architecture-specific ones
MULTIBOOT_OBJS :=\
    $(MULTIBOOT_LOCALDIR)/multiboot.o

CLEAN_OBJS += $(MULTIBOOT_OBJS) $(MULTIBOOT_LOCALDIR)/libmultiboot.a


$(MULTIBOOT_LOCALDIR)/multiboot.o : $(MULTIBOOT_LOCALDIR)/multiboot.c
	$(CC) -MD -c $< -o $@ $(CFLAGS) -Wno-unused-variable $(CPPFLAGS) 

install-multiboot-headers: 
	mkdir -p $(BUILD)/$(MULTIBOOT_BUILD_INCLUDE_INSTALL_DIR)
	cp -r $(MULTIBOOT_LOCALDIR)/include/* $(BUILD)/$(MULTIBOOT_BUILD_INCLUDE_INSTALL_DIR)

libmultiboot: install-multiboot-headers $(MULTIBOOT_OBJS)
	@echo "\nBuilding $@"
	$(AR) rcs $(MULTIBOOT_LOCALDIR)/$@.a $(MULTIBOOT_OBJS)
	mkdir -p $(BUILD)/$(MULTIBOOT_BUILD_INSTALL_DIR)
	cp $(MULTIBOOT_LOCALDIR)/$@.a $(BUILD)/$(MULTIBOOT_BUILD_INSTALL_DIR)
