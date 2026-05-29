include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/package.mk
include ../sunxifb.mk

PKG_NAME:=lv_nes
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
SRC_CODE_DIR := ./src
define Package/$(PKG_NAME)
  SECTION:=gui
  SUBMENU:=Littlevgl
  CATEGORY:=Gui
  DEPENDS:=+LVGL8_USE_SUNXIFB_G2D:libuapi +LVGL8_USE_SUNXIFB_G2D:kmod-sunxi-g2d \
           +LVGL8_USE_FREETYPE:libfreetype
  TITLE:=lvgl nes demo
endef

PKG_CONFIG_DEPENDS := \
	CONFIG_LVGL8_USE_SUNXIFB_DOUBLE_BUFFER \
	CONFIG_LVGL8_USE_SUNXIFB_CACHE 

define Package/$(PKG_NAME)/config
endef

define Package/$(PKG_NAME)/Default
endef

define Package/$(PKG_NAME)/description
  a lvgl nes v8.1.0
endef

define Build/Prepare
	$(INSTALL_DIR) $(PKG_BUILD_DIR)/
	$(CP) -r $(SRC_CODE_DIR)/ $(PKG_BUILD_DIR)/
	$(CP) -r $(LICHEE_PLATFORM_DIR)/thirdparty/gui/lvgl-8/lvgl $(PKG_BUILD_DIR)/src/
	$(CP) -r $(LICHEE_PLATFORM_DIR)/thirdparty/gui/lvgl-8/lv_drivers $(PKG_BUILD_DIR)/src/
endef

define Build/Configure
endef

TARGET_CFLAGS+=-I$(PKG_BUILD_DIR)/src

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/src\
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		INSTALL_PREFIX="$(PKG_INSTALL_DIR)" \
		-j32 \
		all
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/$(PKG_NAME) $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/nes/rom/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/dummy.nes $(1)/etc/nes/rom/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
