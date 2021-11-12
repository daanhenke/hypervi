DIR_ISO := $(DIR_BUILD)/iso

ISO_FILES := \
	EFI/BOOTX64/BOOTX64.EFI

ISO_PATHS := $(addprefix $(DIR_ISO)/,$(ISO_FILES))
$(eval $(call add_targets,$(ISO_PATHS)))

$(DIR_ISO)/EFI/BOOTX64/BOOTX64.EFI: $(EFI_TGT)
	$(call log,iso,Copying $(notdir $@))
	$(call ensure_dir)
	@cp $< $@
