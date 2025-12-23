#!/bin/bash
# mac/linux build steps
# install by using homebrew on mac, and probably apt-get on linux
# macOS: brew update && brew install arduino-cli
# https://docs.arduino.cc/arduino-cli/installation/

set -e  # Exit on error

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
SRC_DIR="$PROJECT_DIR/src/flipper-http"
BOARDS_FILE="$SRC_DIR/boards.hpp"
SKETCH_FILE="$SRC_DIR/flipper-http.ino"
OUTPUT_DIR="$PROJECT_DIR/WiFi Developer Board (ESP32S2)"

# Arduino CLI configuration
export ARDUINO_BOARD_MANAGER_ADDITIONAL_URLS="https://github.com/earlephilhower/arduino-pico/releases/download/5.4.4/package_rp2040_index.json https://github.com/Ameba-AIoT/ameba-arduino-d/raw/dev/Arduino_package/package_realtek_amebad_early_index.json https://dl.espressif.com/dl/package_esp32_index.json"

# Board settings for WiFi Developer Board (ESP32S2 Dev Module)
FQBN="esp32:esp32:esp32s2"
BUILD_PROPS="PartitionScheme=min_spiffs,UploadSpeed=115200"

echo "=== FlipperHTTP Compiler for WiFi Developer Board (ESP32S2) ==="
echo ""

# Check if arduino-cli is installed
if ! command -v arduino-cli &> /dev/null; then
    echo "Error: arduino-cli is not installed."
    echo "Install it with: brew install arduino-cli (macOS) or follow https://docs.arduino.cc/arduino-cli/installation/"
    exit 1
fi

# Update board index and install ESP32 core if needed
echo "Updating Arduino CLI cores..."
arduino-cli core update-index
if ! arduino-cli core list | grep -q "esp32:esp32"; then
    echo "Installing ESP32 core..."
    arduino-cli core install esp32:esp32
else
    echo "ESP32 core already installed."
fi

# Install required libraries
echo "Installing required libraries..."
arduino-cli lib install "ArduinoJson" "ArduinoHttpClient" || true

# Backup original boards.hpp
cp "$BOARDS_FILE" "$BOARDS_FILE.bak"

echo "Configuring boards.hpp for BOARD_WIFI_DEV..."

# Comment out all board definitions first, then uncomment BOARD_WIFI_DEV
sed -i.tmp 's/^#define BOARD_/\/\/ #define BOARD_/' "$BOARDS_FILE"
sed -i.tmp 's/^\/\/ #define BOARD_WIFI_DEV/#define BOARD_WIFI_DEV/' "$BOARDS_FILE"
rm -f "$BOARDS_FILE.tmp"

echo "Compiling sketch..."
arduino-cli compile \
    --fqbn "$FQBN" \
    --build-property "build.extra_flags=-DBOARD_WIFI_DEV=0 -DESP32" \
    --board-options "$BUILD_PROPS" \
    --output-dir "$OUTPUT_DIR" \
    "$SRC_DIR"

# Restore original boards.hpp
mv "$BOARDS_FILE.bak" "$BOARDS_FILE"

echo "Organizing output files..."

# Rename and keep only the required files
cd "$OUTPUT_DIR"

# Rename the files to the required names
if [ -f "flipper-http.ino.partitions.bin" ]; then
    mv "flipper-http.ino.partitions.bin" "flipper_http_partitions.bin"
    echo "✓ Created flipper_http_partitions.bin"
fi

if [ -f "flipper-http.ino.bootloader.bin" ]; then
    mv "flipper-http.ino.bootloader.bin" "flipper_http_bootloader.bin"
    echo "✓ Created flipper_http_bootloader.bin"
fi

if [ -f "flipper-http.ino.bin" ]; then
    mv "flipper-http.ino.bin" "flipper_http_firmware_a.bin"
    echo "✓ Created flipper_http_firmware_a.bin"
fi

if [ -f "flipper-http.ino.merged.bin" ]; then
    mv "flipper-http.ino.merged.bin" "flipper_http_merged.bin"
    echo "✓ Created flipper_http_merged.bin"
fi

# Remove all other files (keep only the 4 required .bin files)
find . -type f ! -name "flipper_http_partitions.bin" \
              ! -name "flipper_http_bootloader.bin" \
              ! -name "flipper_http_firmware_a.bin" \
              ! -name "flipper_http_merged.bin" \
              ! -name "README.md" \
              -delete

echo ""
echo "=== Compilation Complete ==="
echo "Output files in: $OUTPUT_DIR"
echo "  - flipper_http_partitions.bin"
echo "  - flipper_http_bootloader.bin"
echo "  - flipper_http_firmware_a.bin"
echo "  - flipper_http_merged.bin"
echo ""
