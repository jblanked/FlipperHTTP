#pragma once
#include <Arduino.h>
#include "boards.hpp"
#ifndef BOARD_BW16
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
typedef struct
{
    String ssid;
    short rssi;
    uint8_t channel;
} WiFiScanResult;

// Maximum number of APs to record in a single scan
#define WIFI_MAX_SCAN 64

// Global scan results array and count
extern WiFiScanResult wifiScanResults[WIFI_MAX_SCAN];
extern int wifiScanCount;
#else
#include "WiFiClient.h"
#include "WiFiServer.h"
#include <wifi_conf.h>
#define WIFI_STA RTW_MODE_STA
#define WIFI_AP RTW_MODE_AP

typedef struct
{
    String ssid;
    String bssid_str;
    uint8_t bssid[6];
    short rssi;
    uint8_t channel;
    rtw_security_t security;
} WiFiScanResult;

// Maximum number of APs to record in a single scan
#define BW16_MAX_SCAN 64

// Global scan results array and count
extern WiFiScanResult bw16ScanResults[BW16_MAX_SCAN];
extern int bw16ScanCount;
#endif
#include "WiFi.h"

class WiFiUtils
{
public:
    WiFiUtils()
    {
    }
    bool connect(const char *ssid, const char *password); // Connect to WiFi using the provided SSID and password
    String connectAP(const char *ssid);                   // Connect to WiFi in AP mode and return the IP address
    String deviceIP();                                    // Get IP address of the device
    void disconnect();                                    // Disconnect from WiFi
    String getSSID();                                     // Get the name of the connected network
    bool isConnected();                                   // Check if connected to WiFi
    String scan();                                        // Scan for available WiFi networks
private:
    bool connectHelper(const char *ssid, const char *password, bool isAP = false); // Helper function to connect to WiFi
};