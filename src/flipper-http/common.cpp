#include "common.hpp"
#include "boards.hpp"

const char *commonGetBoardName()
{
    // board name
#if defined(BOARD_WIFI_DEV)
    return "WiFi Developer Board";
#elif defined(BOARD_ESP32_C6)
    return "ESP32-C6";
#elif defined(BOARD_ESP32_CAM)
    return "ESP32-Cam";
#elif defined(BOARD_ESP32_S3)
    return "ESP32-S3";
#elif defined(BOARD_ESP32_WROOM)
    return "ESP32-WROOM";
#elif defined(BOARD_ESP32_WROVER)
    return "ESP32-WROVER";
#elif defined(BOARD_PICO_W)
    return "Pico W";
#elif defined(BOARD_PICO_2W)
    return "Pico 2W";
#elif defined(BOARD_VGM)
    return "Video Game Module";
#elif defined(BOARD_ESP32_C3)
    return "ESP32-C3";
#elif defined(BOARD_BW16)
    return "BW16";
#elif defined(BOARD_ESP32_C5)
    return "ESP32-C5";
#elif defined(BOARD_PICOCALC_W)
    return "PicoCalc W";
#elif defined(BOARD_PICOCALC_2W)
    return "PicoCalc 2W";
#else
    return "Unknown Board";
#endif
}

size_t commonGetFreeHeap()
{
#if defined(BOARD_PICO_W) || defined(BOARD_PICO_2W) || defined(BOARD_VGM) || defined(BOARD_PICOCALC_W) || defined(BOARD_PICOCALC_2W)
    return rp2040.getFreeHeap();
#elif defined(BOARD_BW16)
    return os_get_free_heap_size_arduino();
#else
    return ESP.getFreeHeap();
#endif
}

void commonReboot()
{
#if defined(BOARD_PICO_W) || defined(BOARD_PICO_2W) || defined(BOARD_VGM) || defined(BOARD_PICOCALC_W) || defined(BOARD_PICOCALC_2W)
    rp2040.reboot();
#elif defined(BOARD_BW16)
    ota_platform_reset();
#else
    ESP.restart();
#endif
}