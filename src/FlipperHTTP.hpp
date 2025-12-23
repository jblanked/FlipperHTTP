/* FlipperHTTP.h for flipper-http.ino
Author: JBlanked
Github: https://github.com/jblanked/FlipperHTTP
Info: This library is a wrapper around the HTTPClient library and is used to communicate with the FlipperZero over serial.
Created: 2024-09-30
Updated: 2025-12-22

Change Log:
- 2024-09-30: Initial commit
.
.
.
- 2024-10-16: Fixed typos and added [GET/BYTES], [POST/BYTES], and [WIFI/SACN] commands
- 2024-10-17: Added [LIST], [REBOOT], [PARSE], [PARSE/ARRAY], [LED/ON], and [LED/OFF], and [IP/ADDRESS] commands
- 2024-10-19: Added [WIFI/IP] command
- 2024-10-21: Removed unnecessary println
- 2024-10-22: Updated Post Bytes and Get Bytes methods
- 2024-10-25: Updated to automatically connect to WiFi on boot if settings are saved
- 2024-10-26: Updated the saveWifiSettings and loadWifiSettings methods to save and load a list of wifi networks, and added [WIFI/LIST] command
- 2024-11-09: Added SSL certificate from https://letsencrypt.org/certificates/
- 2024-12-02: Restructured the code
- 2025-01-12:
    - Added uploadBytes method
    - Added timeout to stream requests
    - Added NTP time sync
- 2025-01-19:
    - Created a LED class to handle LED actions for all supported boards
    - Created an UART class to handle Serial actions for all supported boards
    - Combined source files from all supported boards into one file
- 2025-02-03: Added WebSockets support: [SOCKET/START] and [SOCKET/STOP]
- 2025-02-16: Added support for the ESP32-C3 board (@dj1ch)
- 2025-02-21: Simplified HTTP requests
- 2025-02-22: Update the request response to include the status code and content length
- 2025-03-11: Updated certs and support for the BW16 board
- 2025-03-25: Check if websocket is connected
- 2025-03-26: Updated websocket setup
- 2025-03-29: Created a WiFiUtils class to handle WiFi functions (wifi_utils.h/cpp)
- 2025-04-12: Added AP mode support [WIFI/AP] (wifi_ap.h/cpp)
- 2025-04-25:
    - Removed the uploadBytes method
    - Added [VERSION] command to get the version of the library
    - Handled ArduinoJson deprecation warnings
- 2025-04-26: Updated AP mode to redirect clients to the captive portal
- 2025-04-30: Added support for the ESP32-C5 board
- 2025-05-03: Added deauth support for ESP32 and BW16 boards
- 2025-05-23: Changed from .h to .hpp file extension
- 2025-08-19:
    - Updated FlipperHTTP to version 2.0.1
    - Added sendLargeMessage method to handle websocket messages bigger than 90 bytes (128 was the previous limit)
    - Added [WIFI/STATUS] command which returns true if the device is connected to WiFi
    - Added BOARD_PICOCALC_W and BOARD_PICOCALC_2W for PicoCalc support
    - Added LCD class for the PicoCalc
    - Bumped version to 2.1
- 2025-09-15:
    - Added getSSID() method to WiFiUtils class
    - [WIFI/SSID] command to get the SSID of the connected network
    - Bumped version to 2.1.1
- 2025-12-22:
    - Increased MAX_CHUNK_SIZE to 128 bytes
    - Bumped rp2040 package to 5.4.4
    - Bumped version to 2.1.2
*/
#pragma once
#include "certs.hpp"
#include "led.hpp"
#include "uart.hpp"
#include "wifi_utils.hpp"
#include <ArduinoJson.h>
#include <Arduino.h>
#include <ArduinoHttpClient.h>
#include <stdint.h>
#include <string.h>
#include "storage.hpp"

#define BAUD_RATE 115200
#define FLIPPER_HTTP_VERSION "2.1.1"

class FlipperHTTP
{
public:
    // Constructor
    FlipperHTTP()
    {
    }

    bool loadWiFi(); // Load Wifi settings from storage
    //
    String request(
        const char *method,                   // HTTP method
        String url,                           // URL to send the request to
        String payload = "",                  // Payload to send with the request
        const char *headerKeys[] = nullptr,   // Array of header keys
        const char *headerValues[] = nullptr, // Array of header values
        int headerSize = 0                    // Number of headers
    );
    //
    bool saveWiFi(String data); // Save and Load settings to and from storage
    void setup();
    void sendLargeMessage(WebSocketClient &ws, String message);                                                                             // Arduino setup function
    bool streamBytes(const char *method, String url, String payload, const char *headerKeys[], const char *headerValues[], int headerSize); // Stream bytes from server
    bool readSerialSettings(String receivedData, bool connectAfterSave);                                                                    // Read the serial data and save the settings
    void loop();                                                                                                                            // Main loop for flipper-http.ino that handles all of the commands
private:
    char loaded_ssid[64] = {0}; // Variable to store SSID
    char loaded_pass[64] = {0}; // Variable to store password
    bool use_led = true;        // Variable to control LED usage
#ifndef BOARD_BW16
    WiFiClientSecure client; // WiFiClientSecure object for secure connections
#else
    WiFiSSLClient client; // WiFiClient object for secure connections
#endif
    LED led; // EasyLED object to control the LED

#ifdef BOARD_VGM
    UART *uart;   // UART object to handle serial communication
    UART *uart_2; // UART object to handle serial communication
#else
    UART *uart; // UART object to handle serial communication
#endif
    WiFiUtils wifi;         // WiFiUtils object to handle WiFi connections
    StorageManager storage; // StorageManager object to handle storage operations
};

const PROGMEM char settingsFilePath[] = "/flipper-http.json"; // Path to the settings file in the SPIFFS file system
