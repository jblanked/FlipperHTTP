#include "wifi_utils.hpp"

#ifndef BOARD_BW16
WiFiScanResult wifiScanResults[WIFI_MAX_SCAN];
int wifiScanCount = 0;
#else
WiFiScanResult bw16ScanResults[BW16_MAX_SCAN];
int bw16ScanCount = 0;
// scan callback
static rtw_result_t scanResultHandler(rtw_scan_handler_result_t *scan_result)
{
    if (scan_result->scan_complete == 0 && bw16ScanCount < BW16_MAX_SCAN)
    {
        auto *rec = &scan_result->ap_details;

        // Null-terminate SSID
        rec->SSID.val[rec->SSID.len] = '\0';

        // Populate our slot
        auto &slot = bw16ScanResults[bw16ScanCount++];
        slot.ssid = String((char *)rec->SSID.val);
        slot.channel = rec->channel;
        slot.rssi = rec->signal_strength;
        memcpy(slot.bssid, rec->BSSID.octet, 6);

        // Format BSSID as XX:XX:XX:XX:XX:XX
        char buf[18];
        snprintf(buf, sizeof(buf),
                 "%02X:%02X:%02X:%02X:%02X:%02X",
                 rec->BSSID.octet[0], rec->BSSID.octet[1],
                 rec->BSSID.octet[2], rec->BSSID.octet[3],
                 rec->BSSID.octet[4], rec->BSSID.octet[5]);
        slot.bssid_str = String(buf);

        // Capture the security enum
        slot.security = rec->security;
    }
    return RTW_SUCCESS;
}

// Kick off a full scan, filling bw16ScanResults[]
static bool bw16Scan(int timeout = 10)
{
    bw16ScanCount = 0;
    if (wifi_scan_networks(scanResultHandler, NULL) == RTW_SUCCESS)
    {
        delay(timeout * 1000);
        return true;
    }
    return false;
}
#endif

bool WiFiUtils::connectHelper(const char *ssid, const char *password, bool isAP)
{
    if (isAP && strlen(ssid) == 0)
    {
        return false;
    }
    if (!isAP && (strlen(ssid) == 0 || strlen(password) == 0))
    {
        return false;
    }

#ifdef BOARD_ESP32_C5
    WiFi.setBandMode(WIFI_BAND_MODE_AUTO);
#endif

    if (isAP)
    {
#ifndef BOARD_BW16
        WiFi.mode(WIFI_AP);
        WiFi.softAP(ssid);
#else
        WiFi.apbegin((char *)ssid, "", "1");
#endif
    }
    else
    {
#ifndef BOARD_BW16
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid, password);
#else
        WiFi.begin((char *)ssid, password);
#endif
    }
#ifdef BOARD_ESP32_C3
    WiFi.setTxPower(WIFI_POWER_8_5dBm);
#endif
    if (!isAP)
    {
#if !defined(BOARD_BW16) && !defined(BOARD_PICO_W) && !defined(BOARD_PICO_2W) && !defined(BOARD_VGM) && !defined(BOARD_PICOCALC_W) && !defined(BOARD_PICOCALC_2W)
        WiFi.setAutoReconnect(false);
#endif
        int i = 0;
        while (!this->isConnected() && i < 20)
        {
            delay(500);
            i++;
        }
        return this->isConnected();
    }
    return true;
}

bool WiFiUtils::connect(const char *ssid, const char *password)
{
    if (this->connectHelper(ssid, password))
    // Set the time zone to UTC+0
    {
#ifndef BOARD_BW16
        configTime(0, 0, "pool.ntp.org", "time.nist.gov");
#endif
        return true;
    }
    return false;
}

String WiFiUtils::connectAP(const char *ssid)
{
    if (!this->connectHelper(ssid, "", true))
    {
        return "";
    }
#ifndef BOARD_BW16
    return WiFi.softAPIP().toString();
#else
    return WiFi.localIP().get_address();
#endif
}

String WiFiUtils::deviceIP()
{
#ifndef BOARD_BW16
    return WiFi.localIP().toString();
#else
    return WiFi.localIP().get_address();
#endif
}

void WiFiUtils::disconnect()
{
#ifndef BOARD_BW16
    WiFi.disconnect(true);
#else
    WiFi.disconnect();
#endif
}

String WiFiUtils::getSSID()
{
    if (this->isConnected())
    {
        return WiFi.SSID();
    }
    return "";
}

bool WiFiUtils::isConnected()
{
    return WiFi.status() == WL_CONNECTED;
}

String WiFiUtils::scan()
{
#ifdef BOARD_BW16
    // reset the results and count
    bw16ScanCount = 0;
    for (int i = 0; i < BW16_MAX_SCAN; i++)
    {
        bw16ScanResults[i].ssid = "";
        bw16ScanResults[i].bssid_str = "";
        bw16ScanResults[i].rssi = 0;
        bw16ScanResults[i].channel = 0;
    }
    if (!bw16Scan())
    {
        Serial1.println(F("[ERROR] Failed to scan for networks."));
        return "";
    }
#else
    WiFi.scanDelete();
    int n = WiFi.scanNetworks();
#endif
    String json = "{\"networks\":[";
#ifndef BOARD_BW16
    for (int i = 0; i < n; ++i)
    {
        json += "\"";
        json += WiFi.SSID(i);
        wifiScanResults[i].ssid = WiFi.SSID(i);
        wifiScanResults[i].rssi = WiFi.RSSI(i);
        wifiScanResults[i].channel = WiFi.channel(i);
        wifiScanCount++;
        json += "\"";
        if (i < n - 1)
        {
            json += ",";
        }
    }
#else
    for (int i = 0; i < bw16ScanCount; ++i)
    {
        json += "\"";
        json += bw16ScanResults[i].ssid;
        json += "\"";
        if (i < bw16ScanCount - 1)
        {
            json += ",";
        }
    }
#endif
    json += "]}";
    return json;
}