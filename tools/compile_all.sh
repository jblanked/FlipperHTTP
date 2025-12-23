#!/bin/bash
# mac/linux build steps
# install by using homebrew on mac, and probably apt-get on linux
# macOS: brew update && brew install arduino-cli
# https://docs.arduino.cc/arduino-cli/installation/

set +e  # Don't exit on error, we want to compile all boards

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "╔════════════════════════════════════════════════════════════════╗"
echo "║        FlipperHTTP - Compile All Boards                       ║"
echo "╚════════════════════════════════════════════════════════════════╝"
echo ""

START_TIME=$(date +%s)

# Arrays to track results
declare -a SUCCESSFUL=()
declare -a FAILED=()

# Function to compile a board
compile_board() {
    local script=$1
    local board_name=$2
    
    echo ""
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo "  Compiling: $board_name"
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    
    if [ -f "$SCRIPT_DIR/$script" ]; then
        if "$SCRIPT_DIR/$script"; then
            SUCCESSFUL+=("$board_name")
            echo "SUCCESS: $board_name"
        else
            FAILED+=("$board_name")
            echo "FAILED: $board_name"
        fi
    else
        FAILED+=("$board_name (script not found)")
        echo "FAILED: $board_name (script not found)"
    fi
}

# Compile all ESP32 boards
compile_board "compile_devboard.sh" "WiFi Developer Board (ESP32S2)"
compile_board "compile_esp32_c3.sh" "ESP32-C3"
compile_board "compile_esp32_c5.sh" "ESP32-C5"
compile_board "compile_esp32_c6.sh" "ESP32-C6"
compile_board "compile_esp32_cam.sh" "ESP32-Cam"
compile_board "compile_esp32_s3.sh" "ESP32-S3"
compile_board "compile_esp32_wroom.sh" "ESP32-WROOM"
compile_board "compile_esp32_wrover.sh" "ESP32-WROVER"

# Compile all Raspberry Pi Pico boards
compile_board "compile_pico_w.sh" "Raspberry Pi Pico W"
compile_board "compile_pico_2w.sh" "Raspberry Pi Pico 2W"
compile_board "compile_vgm.sh" "Video Game Module"
compile_board "compile_picocalc_w.sh" "PicoCalc W"
compile_board "compile_picocalc_2w.sh" "PicoCalc 2W"

END_TIME=$(date +%s)
ELAPSED=$((END_TIME - START_TIME))
MINUTES=$((ELAPSED / 60))
SECONDS=$((ELAPSED % 60))

# Print summary
echo ""
echo "╔════════════════════════════════════════════════════════════════╗"
echo "║                    Compilation Summary                         ║"
echo "╚════════════════════════════════════════════════════════════════╝"
echo ""
echo "Total time: ${MINUTES}m ${SECONDS}s"
echo ""

if [ ${#SUCCESSFUL[@]} -gt 0 ]; then
    echo "Successful (${#SUCCESSFUL[@]}):"
    for board in "${SUCCESSFUL[@]}"; do
        echo "   • $board"
    done
    echo ""
fi

if [ ${#FAILED[@]} -gt 0 ]; then
    echo "Failed (${#FAILED[@]}):"
    for board in "${FAILED[@]}"; do
        echo "   • $board"
    done
    echo ""
    exit 1
else
    echo "All boards compiled successfully!"
    echo ""
    exit 0
fi
