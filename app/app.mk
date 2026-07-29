APP_VERSION = 1.0
APP_SITE = $(TOPDIR)/../ledgpio/app
APP_SITE_METHOD = local

define APP_BUILD_CMDS
    $(TARGET_CC) $(TARGET_CFLAGS) -o $(@D)/app $(APP_SITE)/app.c
endef

define APP_INSTALL_TARGET_CMDS
    $(INSTALL) -D -m 0755 $(@D)/app $(TARGET_DIR)/usr/bin/app
endef

$(eval $(generic-package))
                              