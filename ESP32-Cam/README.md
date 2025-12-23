## ESP32-Cam Installation
1. Install the ESP Flasher app on your Flipper Zero from the Apps catalog: https://lab.flipper.net/apps/esp_flasher
2. Download the `flipper_http_bootloader_esp32_cam.bin`, `flipper_http_firmware_a_esp32_cam.bin`, and `flipper_http_partitions_esp32_cam.bin` files.
3. Disconnect your ESP32-CAM and connect your Flipper Zero to your computer.
4. Open up qFlipper.
5. Click on the `File manager`.
6. Navigate to `SD Card/apps_data/esp_flasher/`. If the folder doesn’t exist, create it yourself or run the ESP Flasher app once.
7. Drag all three bin files you downloaded earlier into the directory.
8. Disconnect your Flipper from your computer then turn off your Flipper.
9. Connect your ESP32-CAM into the Flipper then turn on your Flipper.
10. Open the ESP Flasher app on your Flipper, it should be located in the `Apps->GPIO` folder from the main menu. 
11. In the ESP Flasher app, select the following options:
    - `Reset Board`: wait a few seconds, then go back.
    - `Enter Bootloader`: wait until the 'waiting for download' message appears, then go back.
    - `Flash ESP`: if you do not see this option at the top of the main menu, then click `Manual Flash`.
13. Click on `Bootloader (0x1000)` and select the `flipper_http_bootloader_esp32_cam.bin` that you downloaded earlier.
14. Click on `Part Table (0x8000)` and select the `flipper_http_partitions_esp32_cam.bin` that you downloaded earlier.
15. Click on `FirmwareA (0x10000)` and select the `flipper_http_firmware_a_esp32_cam.bin` that you downloaded earlier.
16. Click on `FLASH - slow` to flash the firmware to your Developer Board. Wait until the process is complete. If successful, the LED of the Developer Board will flash GREEN three times.
17. Lastly, on the Developer Board, press the RESET button once.

You are all set. Check out these video tutorials if you need help: https://github.com/jblanked/FlipperHTTP/tree/main/Tutorials.md

For wiring information, click [here](https://github.com/jblanked/FlipperHTTP?tab=readme-ov-file#wiring).