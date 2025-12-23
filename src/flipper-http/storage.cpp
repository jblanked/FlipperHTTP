#include "storage.hpp"

#define UNUSED(expr)  \
    do                \
    {                 \
        (void)(expr); \
    } while (0)

bool StorageManager::begin()
{
#if defined(BOARD_PICO_W) || defined(BOARD_PICO_2W) || defined(BOARD_VGM) || defined(BOARD_PICOCALC_W) || defined(BOARD_PICOCALC_2W)
    if (!LittleFS.begin())
    {
        if (LittleFS.format())
        {
            if (!LittleFS.begin())
            {
                return false;
            }
        }
        return false;
    }
    return true;
#elif defined(BOARD_BW16)
    // no begin needed
    return true;
#else
    return SPIFFS.begin(true);
#endif
}

bool StorageManager::deserialize(JsonDocument &doc, const char *filename)
{
#if defined(BOARD_PICO_W) || defined(BOARD_PICO_2W) || defined(BOARD_VGM) || defined(BOARD_PICOCALC_W) || defined(BOARD_PICOCALC_2W)
    File file = LittleFS.open(filename, "r");
#elif !defined(BOARD_BW16)
    File file = SPIFFS.open(filename, FILE_READ);
#endif
#ifndef BOARD_BW16
    DeserializationError error = deserializeJson(doc, file);
    file.close();
    return !error; // return no error
#else
    /*We will keep all data at the same address and overwrite for now*/
    UNUSED(filename);
    char buffer[512];
    FlashStorage.get(0, buffer);
    buffer[sizeof(buffer) - 1] = '\0'; // Null-terminate the string
    DeserializationError error = deserializeJson(doc, buffer);
    return !error; // return no error
#endif
}

size_t StorageManager::freeHeap()
{
#if defined(BOARD_PICO_W) || defined(BOARD_PICO_2W) || defined(BOARD_VGM) || defined(BOARD_PICOCALC_W) || defined(BOARD_PICOCALC_2W)
    return rp2040.getFreeHeap();
#elif defined(BOARD_BW16)
    return os_get_free_heap_size_arduino();
#else
    return ESP.getFreeHeap();
#endif
}

String StorageManager::read(const char *filename)
{
    String fileContent = "";
#if defined(BOARD_PICO_W) || defined(BOARD_PICO_2W) || defined(BOARD_VGM) || defined(BOARD_PICOCALC_W) || defined(BOARD_PICOCALC_2W)
    File file = LittleFS.open(filename, "r");
#elif !defined(BOARD_BW16)
    File file = SPIFFS.open(filename, FILE_READ);
#endif
#ifndef BOARD_BW16
    if (file)
    {
        // Read the entire file content
        fileContent = file.readString();
        file.close();
    }
#else
    /*We will keep all data at the same address and overwrite for now*/
    // we wrote in char so we need to read in char, then convert to string
    UNUSED(filename);
    char buffer[512];
    FlashStorage.get(0, buffer);
    buffer[sizeof(buffer) - 1] = '\0'; // Null-terminate the string
    fileContent = String(buffer);
#endif
    return fileContent;
}

bool StorageManager::serialize(JsonDocument &doc, const char *filename)
{
#if defined(BOARD_PICO_W) || defined(BOARD_PICO_2W) || defined(BOARD_VGM) || defined(BOARD_PICOCALC_W) || defined(BOARD_PICOCALC_2W)
    File file = LittleFS.open(filename, "w");
#elif !defined(BOARD_BW16)
    File file = SPIFFS.open(filename, FILE_WRITE);
#endif
#ifndef BOARD_BW16
    if (file)
    {
        serializeJson(doc, file);
        file.close();
        return true;
    }
    return false;
#else
    /*We will keep all data at the same address and overwrite for now*/
    UNUSED(filename);
    char buffer[512];
    size_t len = serializeJson(doc, buffer, sizeof(buffer));
    buffer[len] = '\0'; // Null-terminate the string
    FlashStorage.put(0, buffer);
    return true;
#endif
}

bool StorageManager::write(const char *filename, const char *data)
{
#if defined(BOARD_PICO_W) || defined(BOARD_PICO_2W) || defined(BOARD_VGM) || defined(BOARD_PICOCALC_W) || defined(BOARD_PICOCALC_2W)
    File file = LittleFS.open(filename, "w");
#elif !defined(BOARD_BW16)
    File file = SPIFFS.open(filename, FILE_WRITE);
#endif

#ifndef BOARD_BW16
    if (file)
    {
        file.print(data);
        file.close();
        return true;
    }
    return false;
#else
    /*We will keep all data at the same address and overwrite for now*/
    UNUSED(filename);
    FlashStorage.put(0, data);
    return true;
#endif
}
