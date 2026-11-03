#!/bin/bash
set -euo pipefail
python3 tests/settings_preview.py
SDK=$(xcrun --sdk iphonesimulator --show-sdk-path)
ARCH=$(uname -m)
APP=/tmp/JokerPreview.app
mkdir -p "$APP"
mkdir -p /tmp/joker-minizip
for src in vendor/ZipArchive/minizip/*.c vendor/ZipArchive/minizip/compat/*.c; do
  clang -isysroot "$SDK" -target "$ARCH-apple-ios15.0-simulator" -DHAVE_WZAES -DHAVE_PKCRYPT -DHAVE_ARC4RANDOM_BUF -DHAVE_ICONV -DHAVE_INTTYPES_H -DHAVE_STDINT_H -DHAVE_ZLIB -DZLIB_COMPAT -Ivendor/ZipArchive/minizip -c "$src" -o "/tmp/joker-minizip/$(basename "$src").o"
done
python3 tests/pack_fixtures.py "$APP"
clang -fobjc-arc -Wno-deprecated-declarations -Wno-incompatible-pointer-types -isysroot "$SDK" -target "$ARCH-apple-ios15.0-simulator"  tests/SettingsPreview.m /tmp/joker-minizip/*.o -lz -liconv -framework Security -framework UIKit -framework Foundation -framework Photos -framework PhotosUI  -framework UniformTypeIdentifiers -framework CoreGraphics -framework QuartzCore -framework ImageIO -o "$APP/JokerPreview"
cat > "$APP/Info.plist" <<'EOF'
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0"><dict>
<key>CFBundleIdentifier</key><string>test.joker.preview</string>
<key>CFBundleExecutable</key><string>JokerPreview</string>
<key>CFBundleName</key><string>JokerPreview</string>
<key>CFBundleVersion</key><string>1</string>
<key>CFBundlePackageType</key><string>APPL</string>
<key>LSRequiresIPhoneOS</key><true/>
<key>UIDeviceFamily</key><array><integer>1</integer></array>
<key>UILaunchScreen</key><dict/>
</dict></plist>
EOF
UDID=$(xcrun simctl list devices available -j | python3 -c 'import json,sys;d=json.load(sys.stdin);print(next(x["udid"] for devices in d["devices"].values() for x in devices if "iPhone" in x["name"]))')
xcrun simctl boot "$UDID" || true
xcrun simctl bootstatus "$UDID" -b
xcrun simctl spawn "$UDID" defaults write NSGlobalDomain AppleLanguages -array ar
xcrun simctl spawn "$UDID" defaults write NSGlobalDomain AppleLocale ar_SA
xcrun simctl install "$UDID" "$APP"
set +e
xcrun simctl launch --console "$UDID" test.joker.preview 2>&1 | tee /tmp/joker-preview.log
LAUNCH_STATUS=${PIPESTATUS[0]}
set -e
if [ "$LAUNCH_STATUS" -ne 0 ]; then
  find "$HOME/Library/Logs/DiagnosticReports" -name '*Joker*' -type f -exec tail -c 16000 {} \; || true
  exit "$LAUNCH_STATUS"
fi
! grep -q 'FAIL ' /tmp/joker-preview.log
grep -q 'PASS UIKit grid/category/back/layout/art persistence smoke' /tmp/joker-preview.log
CONTAINER=$(xcrun simctl get_app_container "$UDID" test.joker.preview data)
mkdir -p settings-shots
cp "$CONTAINER/Documents/shots/"*.png settings-shots/
