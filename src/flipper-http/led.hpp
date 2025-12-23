#pragma once
#include <Arduino.h>
#include "boards.hpp"

class LED
{
public:
    LED()
    {
    }
    void blink(int timeout = 250); // Turn on and off the LED
    void start();                  // Display LED sequence
    void on();                     // Turn on LED (Green only)
    void off();                    // Turn off all LEDs
};