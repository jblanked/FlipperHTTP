#pragma once
#include <Arduino.h>
#include "boards.hpp"
#ifndef BOARD_BW16
#include <DNSServer.h>
#endif
#include "wifi_utils.hpp"
#include "uart.hpp"

class WiFiAP
{
public:
    WiFiAP(UART *uartClass, WiFiUtils *wifiUtils);
    bool start(const char *ssid);
    void run();

private:
    void printInputs(const String &request);    // Print inputs from the request
    void updateHTML(const String &htmlContent); // Update the HTML content to be served
    UART *uart;                                 // UART object for serial communication
    WiFiUtils *wifi;                            // WiFiUtils object for WiFi operations
    bool isRunning;                             // Flag to indicate if the AP mode is running
    String html;                                // HTML content to be served
#ifndef BOARD_BW16
    DNSServer dnsServer; // DNS server to redirect all domains to AP IP
    IPAddress apIP;      // AP mode IP address
#endif
};