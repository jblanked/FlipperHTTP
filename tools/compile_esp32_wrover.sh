#!/bin/bash
# Compile FlipperHTTP for ESP32-WROVER
# macOS: brew update && brew install arduino-cli

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
SRC_DIR="$PROJECT_DIR/src/flipper-http"
BOARDS_FILE="$SRC_DIR/boards.hpp"
OUTPUT_DIR="$PROJECT_DIR/ESP32-WROVER"

export ARDUINO_BOARD_MANAGER_ADDITIONAL_URLS="https://github.com/earlephilhower/arduino-pico/releases/download/5.4.4/package_rp2040_index.json https://github.com/Ameba-AIoT/ameba-arduino-d/raw/dev/Arduino_package/package_realtek_amebad_early_index.json https://dl.espressif.com/dl/package_esp32_index.json"

FQBN="esp32:esp32:esp32wrover"
BUILD_PROPS="PartitionScheme=min_spiffs,UploadSpeed=115200"
BOARD_DEF="BOARD_ESP32_WROVER"
BOARD_VAL="5"

echo "=== FlipperHTTP Compiler for ESP32-WROVER ==="
echo ""

if ! command -v arduino-cli &> /dev/null; then
    echo "Error: arduino-cli is not installed."
    echo "Install it with: brew install arduino-cli (macOS)"
    exit 1
fi

echo "Updating Arduino CLI cores..."
arduino-cli core update-index
if ! arduino-cli core list | grep -q "esp32:esp32"; then
    echo "Installing ESP32 core..."
    arduino-cli core install esp32:esp32
fi

echo "Installing required libraries..."
arduino-cli lib install "ArduinoJson" "ArduinoHttpClient" || true

cp "$BOARDS_FILE" "$BOARDS_FILE.bak"

echo "Configuring boards.hpp for $BOARD_DEF..."
sed -i.tmp 's/^#define BOARD_/\/\/ #define BOARD_/' "$BOARDS_FILE"
sed -i.tmp "s/^\/\/ #define $BOARD_DEF/#define $BOARD_DEF/" "$BOARDS_FILE"
rm -f "$BOARDS_FILE.tmp"

echo "Compiling sketch..."
arduino-cli compile \
    --fqbn "$FQBN" \
    --build-property "build.extra_flags=-D$BOARD_DEF=$BOARD_VAL -DESP32" \
    --board-options "$BUILD_PROPS" \
    --output-dir "$OUTPUT_DIR" \
    "$SRC_DIR"

mv "$BOARDS_FILE.bak" "$BOARDS_FILE"

echo "Organizing output files..."
cd "$OUTPUT_DIR"

[ -f "flipper-http.ino.partitions.bin" ] && mv "flipper-http.ino.partitions.bin" "flipper_http_partitions_esp32_wrover.bin" && echo "✓ flipper_http_partitions_esp32_wrover.bin"
[ -f "flipper-http.ino.bootloader.bin" ] && mv "flipper-http.ino.bootloader.bin" "flipper_http_bootloader_esp32_wrover.bin" && echo "✓ flipper_http_bootloader_esp32_wrover.bin"
[ -f "flipper-http.ino.bin" ] && mv "flipper-http.ino.bin" "flipper_http_firmware_a_esp32_wrover.bin" && echo "✓ flipper_http_firmware_a_esp32_wrover.bin"
[ -f "flipper-http.ino.merged.bin" ] && mv "flipper-http.ino.merged.bin" "flipper_http_merged_esp32_wrover.bin" && echo "✓ flipper_http_merged_esp32_wrover.bin"

find . -type f ! -name "flipper_http_partitions_esp32_wrover.bin" \
              ! -name "flipper_http_bootloader_esp32_wrover.bin" \
              ! -name "flipper_http_firmware_a_esp32_wrover.bin" \
              ! -name "flipper_http_merged_esp32_wrover.bin" \
              ! -name "README.md" \
              -delete

echo ""
echo "=== Compilation Complete ==="
echo "Output: $OUTPUT_DIR"
