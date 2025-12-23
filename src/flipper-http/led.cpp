#include "led.hpp"

#if defined(BOARD_ESP32_C6) || defined(BOARD_ESP32_C3) || defined(BOARD_ESP32_C5)
#include <Adafruit_NeoPixel.h>
#if defined(BOARD_ESP32_C6)
constexpr uint8_t LED_PIN = 10;
#elif defined(BOARD_ESP32_C3)
constexpr uint8_t LED_PIN = 8;
#elif defined(BOARD_ESP32_C5)
constexpr uint8_t LED_PIN = 27;
#endif
constexpr uint8_t NUM_LEDS = 1;

extern Adafruit_NeoPixel rgbLed;

struct RGB
{
    uint8_t r, g, b;
};

constexpr RGB COLOR_OFF = {0, 0, 0};
constexpr RGB COLOR_GREEN = {0, 255, 0};
#endif

#if defined(BOARD_PICO_W) || defined(BOARD_PICO_2W) || defined(BOARD_VGM) || defined(BOARD_BW16) || defined(BOARD_PICOCALC_W) || defined(BOARD_PICOCALC_2W)
#define LED_ON HIGH
#define LED_OFF LOW
#else
#define LED_ON LOW
#define LED_OFF HIGH
#endif

#if defined(BOARD_ESP32_C6) || defined(BOARD_ESP32_C5)
Adafruit_NeoPixel rgbLed(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
#endif

void LED::blink(int timeout)
{
#ifdef BOARD_WIFI_DEV
    // turn off RED and BLUE
    digitalWrite(6, LED_OFF); // RED
    digitalWrite(4, LED_OFF); // BLUE

    // turn on/off GREEN
    digitalWrite(5, LED_ON); // GREEN
    delay(timeout);
    digitalWrite(5, LED_OFF); // GREEN
    delay(timeout);
#elif defined(BOARD_ESP32_C6)
    // turn on GREEN
    rgbLed.setPixelColor(0, rgbLed.Color(COLOR_GREEN.r, COLOR_GREEN.g, COLOR_GREEN.b));
    rgbLed.show();
    delay(timeout);
    // turn off GREEN
    rgbLed.setPixelColor(0, rgbLed.Color(COLOR_OFF.r, COLOR_OFF.g, COLOR_OFF.b));
    rgbLed.show();
    delay(timeout);
#elif defined(BOARD_ESP32_CAM)
    // turn on GREEN
    digitalWrite(4, LED_ON); // GREEN
    delay(timeout);
    // turn off GREEN
    digitalWrite(4, LED_OFF); // GREEN
    delay(timeout);
#elif defined(BOARD_ESP32_S3)
#ifdef RGB_BUILTIN
    neopixelWrite(RGB_BUILTIN, 0, RGB_BRIGHTNESS, 0); // Green
    delay(timeout);
    neopixelWrite(RGB_BUILTIN, 0, 0, 0); // Off / black
    delay(timeout);
#endif
#elif defined(BOARD_ESP32_WROOM)
    // turn on GREEN
    digitalWrite(2, LED_ON); // GREEN
    delay(timeout);
    // turn off GREEN
    digitalWrite(2, LED_OFF); // GREEN
    delay(timeout);
#elif defined(BOARD_ESP32_WROVER)
    // turn off RED and BLUE
    digitalWrite(6, LED_OFF); // RED
    digitalWrite(4, LED_OFF); // BLUE

    // turn on/off GREEN
    digitalWrite(5, LED_ON); // GREEN
    delay(timeout);
    digitalWrite(5, LED_OFF); // GREEN
    delay(timeout);
#elif defined(BOARD_PICO_W) || defined(BOARD_PICO_2W) || defined(BOARD_VGM) || defined(BOARD_PICOCALC_W) || defined(BOARD_PICOCALC_2W)
    // turn on GREEN
    digitalWrite(LED_BUILTIN, LED_ON);
    delay(timeout);
    // turn off GREEN
    digitalWrite(LED_BUILTIN, LED_OFF);
    delay(timeout);
#elif defined(BOARD_ESP32_C3)
    // turn on GREEN
    digitalWrite(8, LED_ON); // GREEN
    delay(timeout);
    // turn off GREEN
    digitalWrite(8, LED_OFF); // GREEN
    delay(timeout);
#elif defined(BOARD_BW16)
    // turn on GREEN
    digitalWrite(LED_B, LED_ON);
    delay(timeout);
    // turn off GREEN
    digitalWrite(LED_B, LED_OFF);
    delay(timeout);
#elif defined(BOARD_ESP32_C5)
    // turn on GREEN
    rgbLed.setPixelColor(0, rgbLed.Color(COLOR_GREEN.r, COLOR_GREEN.g, COLOR_GREEN.b));
    rgbLed.show();
    delay(timeout);
    // turn off GREEN
    rgbLed.setPixelColor(0, rgbLed.Color(COLOR_OFF.r, COLOR_OFF.g, COLOR_OFF.b));
    rgbLed.show();
    delay(timeout);
#endif
}

void LED::start()
{
#ifdef BOARD_WIFI_DEV
    pinMode(4, OUTPUT); // Set Blue Pin mode as output
    pinMode(5, OUTPUT); // Set Green Pin mode as output
    pinMode(6, OUTPUT); // Set Red Pin mode as output

    digitalWrite(4, LED_OFF); // BLUE
    digitalWrite(6, LED_OFF); // RED
#elif defined(BOARD_ESP32_C6)
    rgbLed.begin();
    rgbLed.show();
#elif defined(BOARD_ESP32_CAM)
    pinMode(4, OUTPUT); // Set Green Pin mode as output
#elif defined(BOARD_ESP32_S3)
#ifdef RGB_BUILTIN
    digitalWrite(RGB_BUILTIN, LOW);      // Turn the RGB LED off
    neopixelWrite(RGB_BUILTIN, 0, 0, 0); // Off / black
#endif
#elif defined(BOARD_ESP32_WROOM)
    pinMode(2, OUTPUT); // Set Green Pin mode as output
#elif defined(BOARD_ESP32_WROVER)
    pinMode(4, OUTPUT); // Set Blue Pin mode as output
    pinMode(5, OUTPUT); // Set Green Pin mode as output
    pinMode(6, OUTPUT); // Set Red Pin mode as output

    digitalWrite(4, LED_OFF); // BLUE
    digitalWrite(6, LED_OFF); // RED
    digitalWrite(5, LED_OFF); // GREEN
#elif defined(BOARD_PICO_W) || defined(BOARD_PICO_2W) || defined(BOARD_VGM) || defined(BOARD_PICOCALC_W) || defined(BOARD_PICOCALC_2W)
    pinMode(LED_BUILTIN, OUTPUT); // Set Green Pin mode as output
#elif defined(BOARD_ESP32_C3)
    pinMode(8, OUTPUT); // Set Green Pin mode as output
#elif defined(BOARD_BW16)
    pinMode(LED_B, OUTPUT); // it says blue, but shows up as green
#elif defined(BOARD_ESP32_C5)
    rgbLed.begin();
    rgbLed.show();
#endif
    this->blink();
    this->blink();
    this->blink();
}

void LED::on()
{
#ifdef BOARD_WIFI_DEV
    digitalWrite(4, LED_OFF); // BLUE
    digitalWrite(6, LED_OFF); // RED
    digitalWrite(5, LED_ON);  // GREEN
#elif defined(BOARD_ESP32_C6)
    rgbLed.setPixelColor(0, rgbLed.Color(COLOR_GREEN.r, COLOR_GREEN.g, COLOR_GREEN.b));
    rgbLed.show();
#elif defined(BOARD_ESP32_CAM)
    digitalWrite(4, LED_ON); // GREEN
#elif defined(BOARD_ESP32_S3)
#ifdef RGB_BUILTIN
    neopixelWrite(RGB_BUILTIN, 0, RGB_BRIGHTNESS, 0); // Green
#endif
#elif defined(BOARD_ESP32_WROOM)
    digitalWrite(2, LED_ON); // GREEN
#elif defined(BOARD_ESP32_WROVER)
    digitalWrite(4, LED_OFF); // BLUE
    digitalWrite(6, LED_OFF); // RED
    digitalWrite(5, LED_ON);  // GREEN
#elif defined(BOARD_PICO_W) || defined(BOARD_PICO_2W) || defined(BOARD_VGM) || defined(BOARD_PICOCALC_W) || defined(BOARD_PICOCALC_2W)
    digitalWrite(LED_BUILTIN, LED_ON); // GREEN
#elif defined(BOARD_ESP32_C3)
    digitalWrite(8, LED_ON); // GREEN
#elif defined(BOARD_BW16)
    digitalWrite(LED_B, LED_ON); // GREEN
#elif defined(BOARD_ESP32_C5)
    rgbLed.setPixelColor(0, rgbLed.Color(COLOR_GREEN.r, COLOR_GREEN.g, COLOR_GREEN.b));
    rgbLed.show();
#endif
}

void LED::off()
{
#ifdef BOARD_WIFI_DEV
    digitalWrite(4, LED_OFF); // BLUE
    digitalWrite(5, LED_OFF); // GREEN
    digitalWrite(6, LED_OFF); // RED
#elif defined(BOARD_ESP32_C6)
    rgbLed.setPixelColor(0, rgbLed.Color(COLOR_OFF.r, COLOR_OFF.g, COLOR_OFF.b));
    rgbLed.show();
#elif defined(BOARD_ESP32_CAM)
    digitalWrite(4, LED_OFF); // GREEN
#elif defined(BOARD_ESP32_S3)
#ifdef RGB_BUILTIN
    neopixelWrite(RGB_BUILTIN, 0, 0, 0); // Off / black
#endif
#elif defined(BOARD_ESP32_WROOM)
    digitalWrite(2, LED_OFF); // GREEN
#elif defined(BOARD_ESP32_WROVER)
    digitalWrite(4, LED_OFF); // BLUE
    digitalWrite(5, LED_OFF); // GREEN
    digitalWrite(6, LED_OFF); // RED
#elif defined(BOARD_PICO_W) || defined(BOARD_PICO_2W) || defined(BOARD_VGM) || defined(BOARD_PICOCALC_W) || defined(BOARD_PICOCALC_2W)
    digitalWrite(LED_BUILTIN, LED_OFF); // GREEN
#elif defined(BOARD_ESP32_C3)
    digitalWrite(8, LED_OFF); // GREEN
#elif defined(BOARD_BW16)
    digitalWrite(LED_B, LED_OFF); // GREEN
#elif defined(BOARD_ESP32_C5)
    rgbLed.setPixelColor(0, rgbLed.Color(COLOR_OFF.r, COLOR_OFF.g, COLOR_OFF.b));
    rgbLed.show();
#endif
}