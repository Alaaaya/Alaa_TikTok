TARGET := iphone:clang:latest:15.0
ARCHS = arm64 arm64e
INSTALL_TARGET_PROCESSES = TikTok

include $(THEOS)/makefiles/common.mk

TWEAK_NAME = Alaa_TikTok
Alaa_TikTok_FILES = Tweak.x
Alaa_TikTok_CFLAGS = -fobjc-arc -Wno-unguarded-availability-new -Wno-deprecated-declarations
Alaa_TikTok_FRAMEWORKS = UIKit QuartzCore AVFoundation MetalKit Photos CoreMedia
Alaa_TikTok_LOGOS_DEFAULT_GENERATOR = internal

include $(THEOS_MAKE_PATH)/tweak.mk
