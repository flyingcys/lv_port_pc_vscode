CSRCS += $(wildcard $(LVGL_DIR)/lv_freetype/*.c)

DEPPATH += --dep-path $(LVGL_DIR)/lv_freetype
VPATH += :$(LVGL_DIR)/lv_freetype

CFLAGS += "-I$(LVGL_DIR)/lv_freetype"
CFLAGS += "-I$(LVGL_DIR)/lv_freetype/include/freetype2"
LDFLAGS += "-L$(LVGL_DIR)/lv_freetype/lib"
LDFLAGS += "-lfreetype"