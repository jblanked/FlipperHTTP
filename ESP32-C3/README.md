## ESP32-C3 Installation
1. Install the ESP Flasher app on your Flipper Zero from the Apps catalog: https://lab.flipper.net/apps/esp_flasher
2. Download the `flipper_http_bootloader_esp32_c3.bin`, `flipper_http_firmware_a_esp32_c3.bin`, and `flipper_http_partitions_esp32_c3.bin` files.
3. Disconnect your ESP32-C3 and connect your Flipper Zero to your computer.
4. Open up qFlipper.
5. Click on the `File manager`.
6. Navigate to `SD Card/apps_data/esp_flasher/`. If the folder doesn’t exist, create it yourself or run the ESP Flasher app once.
7. Drag all three bin files you downloaded earlier into the directory.
8. Disconnect your Flipper from your computer then turn off your Flipper.
9. Connect your ESP32-C3 into the Flipper then turn on your Flipper. (Optionally, turn on your Flipper first. Then, hold the `BOOT` button while connecting your ESP32-C3. If you do this, when you get to step 11, you can skip the `Reset Board` and `Enter Bootloader` steps.)
10. Open the ESP Flasher app on your Flipper, it should be located in the `Apps->GPIO` folder from the main menu. 
11. In the ESP Flasher app, select the following options:
    - `Reset Board`: wait a few seconds, then go back.
    - `Enter Bootloader`: wait until the `waiting for download` message appears, then go back.
    - `Flash ESP`: if you do not see this option at the top of the main menu, then click `Manual Flash`.
12. Click on `Select if using S3`.
13. Click on `Bootloader (0x0)` and select the `flipper_http_bootloader_esp32_c3.bin` that you downloaded earlier.
14. Click on `Part Table (0x8000)` and select the `flipper_http_partitions_esp32_c3.bin` that you downloaded earlier.
15. Click on `FirmwareA (0x10000)` and select the `flipper_http_firmware_a_esp32_c3.bin` that you downloaded earlier.
16. Click on `FLASH - fast (S3)` to flash the firmware to your ESP32-C3. Wait until the process is complete. If successful, the LED of the ESP32-C3 will flash GREEN three times. If you're using a Xiao ESP32C3 or any other ESP32-C3 without an LED, no LED will flash.
17. Lastly, on the ESP32-C3, press the RESET button once. Alternatively, you can disconnect and reconnect the board.

You are all set. Check out these video tutorials if you need help: https://github.com/jblanked/FlipperHTTP/tree/main/Tutorials.md

For wiring information, click [here](https://github.com/jblanked/FlipperHTTP?tab=readme-ov-file#wiring).
