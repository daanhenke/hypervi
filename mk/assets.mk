DIR_ASSETS_IN := $(DIR_SOURCE)assets
DIR_ASSETS_OUT := $(DIR_BUILD)/assets

ALL_ASSETS := \
	bitmapfont.png.h \
	logo.png.h
ALL_ASSETS := $(addprefix $(DIR_ASSETS_OUT)/,$(ALL_ASSETS))

.PHONY: assets
assets: $(ALL_ASSETS)
$(eval $(call add_targets,assets))

$(DIR_ASSETS_OUT)/font.bdf: $(DIR_ASSETS_IN)/fonts/ibm18.bdf
	$(call ensure_dir)
	@cp $< $@

$(DIR_ASSETS_OUT)/bitmapfont.png: $(DIR_ASSETS_OUT)/font.bdf
	$(call log,assets,Creating png $(notdir $@))
	$(call ensure_dir)
	@$(DIR_TOOLS)/bdf2png.py $< $@
	@convert $@ -negate -transparent black -threshold 50% $@

$(DIR_ASSETS_OUT)/bitmapfont.bin: $(DIR_ASSETS_OUT)/bitmapfont.png
	$(call log,assets,Creating bin $(notdir $@))
	$(call ensure_dir)
	@cd $(DIR_ASSETS_OUT) && $(DIR_TOOLS)/bin2png.py -d $(notdir $@) /dev/null

$(DIR_ASSETS_OUT)/%.png.h: $(DIR_ASSETS_OUT)/%.png
	@$(DIR_TOOLS)/png2h.py $(basename $(notdir $<)) $< $@

$(DIR_ASSETS_OUT)/%.png.h: $(DIR_ASSETS_IN)/%.png
	@$(DIR_TOOLS)/png2h.py $(basename $(notdir $<)) $< $@

