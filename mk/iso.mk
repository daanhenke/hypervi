DIR_ISO := $(DIR_BUILD)/iso
ISO_TGT := $(DIR_BUILD)/violet.iso

ISO_FILES := \
	EFI/BOOT/BOOTX64.EFI \
	VIOLET/VIOLET.ELF \
	VIOLET/VIOLET.CFG

ISO_PATHS := $(addprefix $(DIR_ISO)/,$(ISO_FILES))
$(eval $(call add_targets,$(ISO_PATHS)))

$(ISO_TGT): $(ISO_PATHS)
	$(call log,iso,Creating iso...)
	[ -f "$@" ] && rm $@
	@mkfs.msdos -C $@ 32000
	sudo mount $@ /mnt
	sudo cp -r $(DIR_ISO)/* /mnt/
	sudo umount /mnt

$(DIR_ISO)/VIOLET/VIOLET.CFG: $(DIR_ASSETS_IN)/violet.cfg
	$(call log,iso,Copying $(notdir $@))
	$(call ensure_dir)
	@cp $< $@

$(DIR_ISO)/VIOLET/VIOLET.ELF: $(HYPERVISOR_TGT)
	$(call log,iso,Copying $(notdir $@))
	$(call ensure_dir)
	@cp $< $@

$(DIR_ISO)/EFI/BOOT/BOOTX64.EFI: $(EFI_TGT)
	$(call log,iso,Copying $(notdir $@))
	$(call ensure_dir)
	@cp $< $@


.PHONY: iso
iso: $(ISO_TGT)
