LD := lld-link

EFI_CFLAGS := \
	$(FREESTANDING_CFLAGS) \
	-I $(DIR_ASSETS_OUT) \
	-target x86_64-pc-win32-coff

EFI_CXXFLAGS := \
	$(FREESTANDING_CXXFLAGS) \
	-I $(DIR_ASSETS_OUT) \
	-target x86_64-pc-win32-coff

EFI_LDFLAGS := \
	-subsystem:efi_application \
	-nodefaultlib \
	-dll \
	-entry:efi_main

EFI_ASMFLAGS := \
	-g -f win64

EFI_TGT=$(DIR_DIST)/loader-efi.efi
$(eval $(call add_targets,$(EFI_TGT)))

EFI_SOURCES := \
	$(FREESTANDING_SOURCES) \
	loader/efi/main.cc \
	loader/efi/gfx.cc \
	loader/efi/fs.cc \
	loader/efi/input.cc \
	loader/efi/console.cc \
	loader/efi/string_utils.cc \
	loader/efi/allocator.cc \
	loader/efi/loader.cc \
	loader/efi/paging.cc \
	loader/efi/gdt.cc \
	loader/efi/idt.cc \
	loader/efi/idt.S
$(eval $(call convert_sources,EFI))

$(DIR_OBJ)/$(current_target_name)/%.cc.o: $(DIR_SOURCE)/%.cc $(DIR_ASSETS_OUT)/bitmapfont.png.h
	$(call log,loader/efi,(CXX) $@)
	$(call ensure_dir)
	@$(CXX) $(EFI_CXXFLAGS) -c $< -o $@

$(DIR_OBJ)/$(current_target_name)/%.c.o: $(DIR_SOURCE)/%.c $(DIR_ASSETS_OUT)/bitmapfont.png.h
	$(call log,loader/efi,(CC) $@)
	$(call ensure_dir)
	@$(CC) $(EFI_CFLAGS) -c $< -o $@

$(DIR_OBJ)/$(current_target_name)/%.S.o: $(DIR_SOURCE)/%.S
	$(call log,loader/efi,(NASM) $@)
	$(call ensure_dir)
	@$(NASM) $(EFI_ASMFLAGS) $< -o $@

$(EFI_TGT): $(EFI_OBJECTS)
	$(call log,loader/efi,(LD) $@)
	$(call ensure_dir)
	@$(LD) $(EFI_LDFLAGS) -out:$@ $^
