define inc_mk
$(foreach MK_MODULE,$1,$(eval include mk/$(MK_MODULE).mk))
endef

define current_mk
$(realpath $(lastword $(MAKEFILE_LIST)))
endef

define current_target_name
$(notdir $(basename $(call current_mk)))
endef

define add_targets
TARGETS := $$(TARGETS) $(1)
endef

define convert_sources
$(1)_OBJECTS := $(addsuffix .o,$(addprefix $(DIR_OBJ)/$(current_target_name)/,$($(1)_SOURCES)))
endef

define log
	@printf "%s\n" "[$(current_target_name)]: $1"
endef

define ensure_dir
	@mkdir -p $(dir $@)
endef