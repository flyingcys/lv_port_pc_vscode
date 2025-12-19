CSRCS += $(wildcard $(LVGL_DIR)/cJSON/*.c)

DEPPATH += --dep-path $(LVGL_DIR)/cJSON
VPATH += :$(LVGL_DIR)/cJSON

CFLAGS += "-I$(LVGL_DIR)/cJSON"
LDFLAGS += "-L$(LVGL_DIR)/cJSON"
