#pragma once
#include <Arduino.h>
#include "boards.hpp"
#include "lcd.hpp"

class UART
{
public:
    UART()
    {
#if defined(BOARD_PICOCALC_W) || defined(BOARD_PICOCALC_2W)
        // Initialize LCD for PicoCalc display
        this->lcd = new LCD();
        if (this->lcd)
        {
            this->lcd->text(0, 0, "Ready...");
        }
#endif
    }
    size_t available();
    void begin(uint32_t baudrate);
    void flush();
    void clearBuffer();
    void print(String str);
    void printf(const char *format, ...);
    void println(String str = "");
    uint8_t read();
    uint8_t readBytes(uint8_t *buffer, size_t size);
    String readSerialLine();
    String readStringUntilString(const String &terminator, uint32_t timeout = 5000);
    void setTimeout(uint32_t timeout);
    void write(const uint8_t *buffer, size_t size);
#ifdef BOARD_VGM
    void set_pins(uint8_t tx_pin, uint8_t rx_pin);
#endif
private:
#if defined(BOARD_PICO_W) || defined(BOARD_PICO_2W) || defined(BOARD_PICOCALC_W) || defined(BOARD_PICOCALC_2W)
    SerialPIO *serial;
#elif defined(BOARD_VGM)
    SerialPIO *serial;
    uint8_t rx_pin;
    uint8_t tx_pin;
#endif
#if defined(BOARD_PICOCALC_W) || defined(BOARD_PICOCALC_2W)
    LCD *lcd; // LCD for PicoCalc display
#endif
};