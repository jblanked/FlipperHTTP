# Development

The firmware was built with the Arduino IDE. Here's how to set it up:

1. Install the Arduino IDE from https://www.arduino.cc/en/software.
2. Download this repository as a .zip file (available here: https://github.com/jblanked/FlipperHTTP/archive/refs/heads/main.zip).
3. Unzip the file and change the folder named `src` to `flipper-http`. Within that `flipper-http` folder, double-click the `flipper-http.ino` file to open it in Arduino IDE.
4. Go to Preferences/Settings, and add the following to the `Additional Boards Manager URLs` field: 
```
https://github.com/earlephilhower/arduino-pico/releases/download/4.7.1/package_rp2040_index.json,https://github.com/Ameba-AIoT/ameba-arduino-d/raw/dev/Arduino_package/package_realtek_amebad_early_index.json,https://dl.espressif.com/dl/package_esp32_index.json
```

5. In `Boards Manager`:
- Search for `rp2040` and install the `Raspberry Pi Pico/RP2040/RP2350` package by `Earl Philhower`.
- Search for `esp32` and install the `ESP32` package by `Espressif Systems`.
- Search for `realtek` and install the `Realtek Ameba Boards (32-bit ARM Cortex)` package by `Ameba-AIoT` (this is not available on Linux).

6. In `Library Manager`:
- Search for `TFT_eSPI` and install the `TFT_eSPI` library by `Bodmer`.
- Search for `ArduinoJson` and install the `ArduinoJson` library by `Benoit Blanchon`.
- Search for `ArduinoHttpClient` and install the `ArduinoHttpClient` library by `Arduino`.
- Search for `FlashStorage` and install the `FlashStorage_RTL8720` library by `Khoi Hoang`.

7. Restart your Arduino IDE. 
8. Hold the `BOOT` button while connecting your `USB` data cable to your board.
9. In `Tools`, click `Board` and then select your specific board type (e.g., `Raspberry Pi Pico W`). Note, the `BW16` is `Ai-Thinker BW16 (RTL8720DN)` and `WiFi Developer Board` is `ESP32S2 Dev Module`
10. Select the serial port to which your board is connected to.

11. If you plan on developing/compiling for the PicoCalc, then in the `User_Setup.h` file in the `TFT_eSPI` library folder (usually located at `../../Documents/Arduino/libraries/TFT_eSPI/User_Setup.h` or `../Arduino/libraries/TFT_eSPI/User_Setup.h`) and:

Replace it with the following code snippet:
```cpp
#define USER_SETUP_ID 60
#define ILI9488_DRIVER
#define TFT_RGB_ORDER TFT_BGR
#define TFT_MISO 12
#define TFT_MOSI 11
#define TFT_SCLK 10
#define TFT_CS 13
#define TFT_DC 14
#define TFT_RST 15
#define TFT_SPI_PORT 1
#define SPI_FREQUENCY 25000000
#define SPI_TOUCH_FREQUENCY 2500000
#define LOAD_GLCD
#define LOAD_FONT2
#define SMOOTH_FONT
#define TFT_INVERSION_ON
```

12. Back in the `Arduino IDE`, in `Tools` menu:
- if you're compiling for a Raspberry Pi Pico device: change the `Flash Size` to `2MB (Sketch: 1984KB, FS: 64KB)` (or `4MB (Sketch: 4032KB, FS: 64KB)` if compiling for a Raspberry Pi Pico 2W device) and `CPU Speed` to `200MHz`.
- if you're compiling for an ESP32 device: change the `Partition Scheme` to `Minimal SPIFFS (... with OTA/190KB SPIFFS)` and `Upload Speed` to `115200`.
- if you're compiling for the BW16, change `Auto Flash Mode` to `Enable` and `Upload Speed` to `921600`.
13. In the `board.hpp` file, set your board type by uncommenting your board definition. For example, for a BW16, the line should look like this:
```cpp
#define BOARD_BW16 10        // AI-Thinker BW16 (RTL8720DN) 
```
instead of
```cpp
// #define BOARD_BW16 10        // AI-Thinker BW16 (RTL8720DN) 
```
14. Finally, click `Sketch` in the menu, then select `Upload`.
