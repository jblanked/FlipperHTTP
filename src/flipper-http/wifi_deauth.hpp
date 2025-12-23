#pragma once
#include "boards.hpp"
class WiFiDeauth
{
public:
    bool start(const char *ssid);
    void stop();
    void update();
};
