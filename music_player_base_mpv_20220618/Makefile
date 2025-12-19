#
# Makefile
#
CC ?= gcc
LVGL_DIR_NAME ?= lvgl
LVGL_DIR ?= ${shell pwd}
     
CFLAGS ?= -O3 -I$(LVGL_DIR)/ 
LDFLAGS ?= -lSDL2 -lm  -lpthread
BIN = demo


#Collect the files to compile
MAINSRC = ./main.c

include $(LVGL_DIR)/lvgl/lvgl.mk
include $(LVGL_DIR)/cJSON/cJSON.mk
include $(LVGL_DIR)/lv_lib_png/lv_lib_png.mk
include $(LVGL_DIR)/lv_lib_gif/lv_lib_gif.mk
include $(LVGL_DIR)/lv_lib_bmp/lv_lib_bmp.mk
include $(LVGL_DIR)/lv_freetype/lv_freetype.mk
include $(LVGL_DIR)/lv_drivers/lv_drivers.mk
include $(LVGL_DIR)/lv_demos/lv_demo.mk

CSRCS +=$(LVGL_DIR)/mouse_cursor_icon.c 

OBJEXT ?= .o

AOBJS = $(ASRCS:.S=$(OBJEXT))
COBJS = $(CSRCS:.c=$(OBJEXT))
CPPOBJS = $(CPPSRCS:.cpp=$(OBJEXT))

MAINOBJ = $(MAINSRC:.c=$(OBJEXT))

SRCS = $(ASRCS) $(CSRCS) $(CPPOBJS) $(MAINSRC)
OBJS = $(AOBJS) $(COBJS) $(CPPOBJS)

## MAINOBJ -> OBJFILES

all: default

%.o: %.cpp
	@$(CC)  $(CFLAGS) -c $< -o $@
	@echo "CC $<"

%.o: %.c
	@$(CC)  $(CFLAGS) -c $< -o $@
	@echo "CC $<"
    
default: $(AOBJS) $(COBJS) $(CPPOBJS) $(MAINOBJ)
	$(CC) -o $(BIN) $(MAINOBJ) $(AOBJS) $(COBJS) $(CPPOBJS) $(LDFLAGS)

clean: 
	rm -f $(BIN) $(AOBJS) $(COBJS) $(CPPOBJS) $(MAINOBJ)

