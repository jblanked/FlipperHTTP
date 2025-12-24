#!/bin/bash
# Compile FlipperHTTP for Raspberry Pi Pico 2W
# macOS: brew update && brew install arduino-cli

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
SRC_DIR="$PROJECT_DIR/src/flipper-http"
BOARDS_FILE="$SRC_DIR/boards.hpp"
OUTPUT_DIR="$PROJECT_DIR/Raspberry Pi Pico 2 W/C++"

export ARDUINO_BOARD_MANAGER_ADDITIONAL_URLS="https://github.com/earlephilhower/arduino-pico/releases/download/5.4.4/package_rp2040_index.json https://github.com/Ameba-AIoT/ameba-arduino-d/raw/dev/Arduino_package/package_realtek_amebad_early_index.json https://dl.espressif.com/dl/package_esp32_index.json"

# Raspberry Pi Pico 2W: Flash Size 4MB (Sketch: 4032KB, FS: 64KB), CPU Speed 200MHz
FQBN="rp2040:rp2040:rpipico2w"
BUILD_PROPS="flash=4194304_65536,freq=200,ipbtstack=ipv4only"
BOARD_DEF="BOARD_PICO_2W"
BOARD_VAL="7"

echo "=== FlipperHTTP Compiler for Raspberry Pi Pico 2W ==="
echo ""

if ! command -v arduino-cli &> /dev/null; then
    echo "Error: arduino-cli is not installed."
    echo "Install it with: brew install arduino-cli (macOS)"
    exit 1
fi

echo "Updating Arduino CLI cores..."
arduino-cli core update-index
if ! arduino-cli core list | grep -q "rp2040:rp2040"; then
    echo "Installing RP2040 core..."
    arduino-cli core install rp2040:rp2040
fi

echo "Installing required libraries..."
arduino-cli lib install "ArduinoJson" "ArduinoHttpClient" || true

echo "Cleaning build cache to force core rebuild..."
rm -rf /Users/user/Library/Caches/arduino/cores/* 2>/dev/null || true
rm -rf /Users/user/Library/Caches/arduino/sketches/489FBC6CE2F3F1EF969FC62D491B59AA 2>/dev/null || true

cp "$BOARDS_FILE" "$BOARDS_FILE.bak"

echo "Configuring boards.hpp for $BOARD_DEF..."
sed -i.tmp 's/^#define BOARD_/\/\/ #define BOARD_/' "$BOARDS_FILE"
sed -i.tmp "s/^\/\/ #define $BOARD_DEF /#define $BOARD_DEF /" "$BOARDS_FILE"
rm -f "$BOARDS_FILE.tmp"

echo "Compiling sketch..."
arduino-cli compile \
    --fqbn "$FQBN" \
    --build-property "build.extra_flags=-DPICO_CYW43_SUPPORTED=1 -DCYW43_PIN_WL_DYNAMIC=1 -D$BOARD_DEF=$BOARD_VAL -DCORE_DEBUG_LEVEL=0 -Os" \
    --board-options "$BUILD_PROPS" \
    --output-dir "$OUTPUT_DIR" \
    "$SRC_DIR"

mv "$BOARDS_FILE.bak" "$BOARDS_FILE"

echo "Organizing output files..."
cd "$OUTPUT_DIR"

# Pico produces .uf2 file
[ -f "flipper-http.ino.uf2" ] && mv "flipper-http.ino.uf2" "flipper_http_pico_2w_c++.uf2" && echo "✓ flipper_http_pico_2w_c++.uf2"

# Remove all other files except the .uf2 and README
find . -type f ! -name "flipper_http_pico_2w_c++.uf2" \
              ! -name "README.md" \
              -delete

echo ""
echo "=== Compilation Complete ==="
echo "Output: $OUTPUT_DIR"
