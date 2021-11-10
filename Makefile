.PHONY: all
all:

include ./mk/macros.mk
include ./mk/base.mk

MK_MODULES := \
	hypervisor \
	loader-efi \
	iso

$(call inc_mk,$(MK_MODULES))
all: $(TARGETS)