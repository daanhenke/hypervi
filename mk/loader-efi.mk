LD := lld-link

EFI_CFLAGS := \
	$(FREESTANDING_CFLAGS) \
	-target x86_64-pc-win32-coff

EFI_LDFLAGS := \
	-subsystem:efi_application \
	-nodefaultlib \
	-dll \
	-entry:efi_main

EFI_TGT=$(DIR_DIST)/loader-efi.efi
$(eval $(call add_targets,$(EFI_TGT)))

EFI_SOURCES := \
	loader/efi/main.cc
$(eval $(call convert_sources,EFI))

$(DIR_OBJ)/$(current_target_name)/%.cc.o: $(DIR_SOURCE)/%.cc
	$(call log,(CXX) $@)
	$(call ensure_dir)
	@$(CXX) $(EFI_CFLAGS) -c $< -o $@

$(EFI_TGT): $(EFI_OBJECTS)
	$(call log,(LD) $@)
	$(call ensure_dir)
	@$(LD) $(EFI_LDFLAGS) -out:$@ $^