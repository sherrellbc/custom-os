LIBS_LOCALDIR:=$(dir $(lastword $(MAKEFILE_LIST)))
#THIS_MAKEFILE_PATH:=$(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST))
#LOCALDIR:=$(shell cd $(dir $(THIS_MAKEFILE_PATH));pwd)

include $(LIBS_LOCALDIR)/libc/Makefile
include $(LIBS_LOCALDIR)/multiboot/Makefile

libs: libc libmultiboot
install-final-libs: install-final-libc
