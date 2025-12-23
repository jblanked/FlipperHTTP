## BW16 Installation
1. Install the Arduino IDE from https://www.arduino.cc/en/software
2. Download this repository as a .zip file (available here: https://github.com/jblanked/FlipperHTTP/archive/refs/heads/main.zip)
3. Unzip the file. 
4. Within the `src/flipper-http` folder, double-click the `flipper-http.ino` file to open it in Arduino IDE.
5. Within Arduino IDE, navigate to the `boards.hpp` file and un-comment the `#define BOARD_BW16 10` line.
6. In Arduino IDE, go to Preferences/Settings and add the following URL to the `Additional Boards Manager URLs` field: `https://github.com/Ameba-AIoT/ameba-arduino-d/raw/dev/Arduino_package/package_realtek_amebad_early_index.json`. Click `OK`.
7. In `Boards Manager`, search for `realtek` and install the `Realtek Ameba Boards (32-bit ARM Cortex)` package. 
8. In `Libraries Manager`, install the following libraries:
- Search for `ArduinoJson` and install the `ArduinoJson` library by `Benoit Blanchon`.
- Search for `ArduinoHttpClient` and install the `ArduinoHttpClient` library by `Arduino`.
- Search for `FlashStorage` and install the `FlashStorage_RTL8720` library by `Khoi Hoang`.
9. Restart your Arduino IDE.
10. Hold the `BURN` button while connecting your USB-C data cable.
11. Select the serial port to which your BW16 is connected and choose `Ai-Thinker BW16 (RTL8720DN)` as your board.
12. Click `Tools` in the menu, select `Auto Flash Mode`, then click `Enable`.
13. Within the `Tools` menu again, select `Upload Speed` and change it to `921600`.
14. Click `Sketch` in the menu, then select `Upload`.
15. After the upload completes, press the `RST` button once. If your board is flashed correctly, the BW16's LED will flash GREEN three times.

You are all set. Check out these video tutorials if you need help: https://github.com/jblanked/FlipperHTTP/tree/main/Tutorials.md

For wiring information, click [here](https://github.com/jblanked/FlipperHTTP?tab=readme-ov-file#wiring).
 
 