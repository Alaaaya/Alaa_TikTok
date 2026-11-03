# Joker release 61 (integer version; package version comes from control)
TARGET := iphone:clang:latest:15.0
ARCHS = arm64 arm64e
INSTALL_TARGET_PROCESSES = TikTok

include $(THEOS)/makefiles/common.mk

TWEAK_NAME = Alaa_TikTok
Alaa_TikTok_FILES = Tweak.x $(wildcard vendor/ZipArchive/minizip/*.c) $(wildcard vendor/ZipArchive/minizip/compat/*.c)
Alaa_TikTok_LIBRARIES = z iconv
Alaa_TikTok_CFLAGS = -DHAVE_WZAES -DHAVE_PKCRYPT -DHAVE_ARC4RANDOM_BUF -DHAVE_ICONV -DHAVE_INTTYPES_H -DHAVE_STDINT_H -DHAVE_ZLIB -DZLIB_COMPAT -Ivendor/ZipArchive/minizip -fobjc-arc -Wno-unguarded-availability-new -Wno-deprecated-declarations
Alaa_TikTok_FRAMEWORKS = UIKit QuartzCore AVFoundation MetalKit Photos PhotosUI UniformTypeIdentifiers ImageIO CoreMedia CoreVideo Security
Alaa_TikTok_LOGOS_DEFAULT_GENERATOR = internal

include $(THEOS_MAKE_PATH)/tweak.mk
