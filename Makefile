.PHONY: all
all:

include ./mk/macros.mk
include ./mk/base.mk

MK_MODULES := \
	hypervisor \
	assets \
	loader-efi \
	iso \
	qemu

$(call inc_mk,$(MK_MODULES))
all: $(TARGETS)
