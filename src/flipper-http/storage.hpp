#pragma once
#include "boards.hpp"
#if defined(BOARD_PICO_W) || defined(BOARD_PICO_2W) || defined(BOARD_VGM) || defined(BOARD_PICOCALC_W) || defined(BOARD_PICOCALC_2W)
#include <LittleFS.h>
#elif defined(BOARD_BW16)
#include <FlashStorage_RTL8720.h> // https://github.com/khoih-prog/FlashStorage_RTL8720
#else
#include <SPIFFS.h>
#endif
#include <ArduinoJson.h>

class StorageManager
{
public:
    bool begin();
    bool deserialize(JsonDocument &doc, const char *filename);
    size_t freeHeap();
    String read(const char *filename);
    bool serialize(JsonDocument &doc, const char *filename);
    bool write(const char *filename, const char *data);
};