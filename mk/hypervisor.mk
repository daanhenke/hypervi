VISOR_LD := ld

HYPERVISOR_CFLAGS := \
	$(FREESTANDING_CFLAGS) \
	-I $(DIR_ASSETS_OUT) \
	-I $(DIR_SOURCE)/external/ia32-doc/out

HYPERVISOR_CXXFLAGS := \
	$(FREESTANDING_CXXFLAGS) \
	-I $(DIR_ASSETS_OUT) \

HYPERVISOR_LDFLAGS := \
	-e visor_main

HYPERVISOR_ASMFLAGS := \
	-g -f elf64

HYPERVISOR_TGT=$(DIR_DIST)/violet.elf
$(eval $(call add_targets,$(HYPERVISOR_TGT)))

HYPERVISOR_SOURCES := \
	hypervisor/main.cc
$(eval $(call convert_sources,HYPERVISOR))

$(DIR_OBJ)/$(current_target_name)/%.cc.o: $(DIR_SOURCE)/%.cc
	$(call log,hypervisor,(CXX) $@)
	$(call ensure_dir)
	@$(CXX) $(EFI_CXXFLAGS) -c $< -o $@

$(DIR_OBJ)/$(current_target_name)/%.c.o: $(DIR_SOURCE)/%.c
	$(call log,hypervisor,(CC) $@)
	$(call ensure_dir)
	@$(CC) $(EFI_CFLAGS) -c $< -o $@

$(DIR_OBJ)/$(current_target_name)/%.S.o: $(DIR_SOURCE)/%.S
	$(call log,hypervisor,(NASM) $@)
	$(call ensure_dir)
	@$(NASM) $(HYPERVISOR_ASMFLAGS) $< -o $@

$(HYPERVISOR_TGT): $(HYPERVISOR_OBJECTS)
	$(call log,hypervisor,(LD) $@)
	$(call ensure_dir)
	@$(VISOR_LD) $(HYPERVISOR_LDFLAGS) -o $@ $^
