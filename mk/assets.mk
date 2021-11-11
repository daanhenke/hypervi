DIR_ASSETS_IN := $(DIR_SOURCE)assets
DIR_ASSETS_OUT := $(DIR_BUILD)/assets

ALL_ASSETS := \
	bitmapfont.png.h
ALL_ASSETS := $(addprefix $(DIR_ASSETS_OUT)/,$(ALL_ASSETS))

.PHONY: assets
assets: $(ALL_ASSETS)
$(eval $(call add_targets,assets))

$(DIR_ASSETS_OUT)/bitmapfont.png: $(DIR_ASSETS_IN)/font.bdf
	$(call log,assets,Creating png $(notdir $@))
	$(call ensure_dir)
	@$(DIR_TOOLS)/monobit/convert.py --overwrite $< $@
	@convert $@ -alpha off -threshold 50% $@

$(DIR_ASSETS_OUT)/bitmapfont.bin: $(DIR_ASSETS_OUT)/bitmapfont.png
	$(call log,assets,Creating bin $(notdir $@))
	$(call ensure_dir)
	@cd $(DIR_ASSETS_OUT) && $(DIR_TOOLS)/bin2png.py -d $(notdir $@)

$(DIR_ASSETS_OUT)/%.h: $(DIR_ASSETS_OUT)/%
	@$(DIR_TOOLS)/bin2h.py $< font_bin > $@
