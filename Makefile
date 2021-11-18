.PHONY: all
all:

include ./mk/macros.mk
include ./mk/base.mk

MK_MODULES := \
	assets \
	loader-stub \
	loader-efi \
	hypervisor \
	iso \
	qemu

$(call inc_mk,$(MK_MODULES))
all: $(TARGETS)
