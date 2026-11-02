#!/bin/bash
set -euo pipefail
python3 tests/settings_preview.py
SDK=$(xcrun --sdk iphonesimulator --show-sdk-path)
ARCH=$(uname -m)
APP=/tmp/JokerPreview.app
mkdir -p "$APP"
clang -fobjc-arc -Wno-deprecated-declarations -Wno-incompatible-pointer-types -isysroot "$SDK" -target "$ARCH-apple-ios15.0-simulator"  tests/SettingsPreview.m -framework UIKit -framework Foundation -framework Photos -framework PhotosUI  -framework UniformTypeIdentifiers -framework CoreGraphics -framework QuartzCore -framework ImageIO -o "$APP/JokerPreview"
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
xcrun simctl install "$UDID" "$APP"
xcrun simctl launch --console "$UDID" test.joker.preview
CONTAINER=$(xcrun simctl get_app_container "$UDID" test.joker.preview data)
mkdir -p settings-shots
cp "$CONTAINER/Documents/shots/"*.png settings-shots/
