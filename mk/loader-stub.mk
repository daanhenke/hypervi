LDRSTUB_TGT=$(DIR_DIST)/loader-exports-stub.so
$(eval $(call add_targets,loader-stub))

LDRSTUB_SOURCES := \
	loader/exports_stubbed.cc
$(eval $(call convert_sources,LDRSTUB))

$(DIR_OBJ)/$(current_target_name)/%.cc.o: $(DIR_SOURCE)/%.cc
	$(call log,loader-stub,(CXX) $@)
	$(call ensure_dir)
	@$(CXX) $(FREESTANDING_CXXFLAGS) -c $< -o $@


$(LDRSTUB_TGT): $(LDRSTUB_OBJECTS)
	$(call log,loader-stub,(LD) $@)
	$(call ensure_dir)
	@$(VISOR_LD) -shared -o $@ $^
	@cp $(LDRSTUB_TGT) $(dir $(LDRSTUB_TGT))/libloader-exports-stub.so

.PHONY: loader-stub
loader-stub: $(LDRSTUB_TGT)
