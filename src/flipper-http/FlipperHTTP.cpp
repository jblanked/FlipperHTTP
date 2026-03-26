/* FlipperHTTP.cpp for flipper-http.ino and FlipperHTTP.h
Author: JBlanked
Github: https://github.com/jblanked/FlipperHTTP
Info: This library is a wrapper around the HTTPClient library and is used to communicate with the FlipperZero over serial.
Created: 2024-09-30
Updated: 2026-03-26
*/

#include "FlipperHTTP.hpp"
#include "wifi_ap.hpp"
#include "wifi_deauth.hpp"
#include "common.hpp"
#include "command.hpp"

#define MAX_CHUNK_SIZE 128

// Load WiFi settings
bool FlipperHTTP::loadWiFi()
{
    JsonDocument doc;
    if (!storage.deserialize(doc, settingsFilePath))
    {
        return false;
    }

    if (!doc["wifi_list"] || !doc["wifi_list"].is<JsonArray>())
    {
        this->uart->println(F("[ERROR] JSON missing 'wifi_list' or it's not an array."));
        return false;
    }

    JsonArray wifiList = doc["wifi_list"].as<JsonArray>();

    for (JsonObject wifi : wifiList)
    {
        // Skip if no SSID or password
        if (!wifi["ssid"] || !wifi["password"])
            continue;

        const char *ssid = wifi["ssid"];
        const char *password = wifi["password"];

        strncpy(loaded_ssid, ssid, sizeof(loaded_ssid));
        strncpy(loaded_pass, password, sizeof(loaded_pass));

        // Try to connect
        if (this->wifi.connect(loaded_ssid, loaded_pass))
        {
            return true;
        }
    }

    this->uart->println(F("[ERROR] No networks connected."));
    return false;
}

// Save WiFi settings to storage
bool FlipperHTTP::saveWiFi(const String jsonData)
{
    JsonDocument newEntryDoc;
    auto err = deserializeJson(newEntryDoc, jsonData);
    if (err)
    {
        this->uart->println(F("[ERROR] Failed to parse JSON data."));
        return false;
    }

    if (!newEntryDoc["ssid"] || !newEntryDoc["password"])
    {
        this->uart->println(F("[ERROR] JSON must contain 'ssid' and 'password'."));
        return false;
    }

    const char *newSSID = newEntryDoc["ssid"];
    const char *newPassword = newEntryDoc["password"];

    JsonDocument settingsDoc;
    bool hadSettings = storage.deserialize(settingsDoc, settingsFilePath);

    JsonArray wifiList;
    if (hadSettings && settingsDoc["wifi_list"] && settingsDoc["wifi_list"].is<JsonArray>())
    {
        // Use the existing array
        wifiList = settingsDoc["wifi_list"].as<JsonArray>();
    }
    else
    {
        // No valid settings on disk yet → clear and create a new array
        settingsDoc.clear();
        wifiList = settingsDoc["wifi_list"].to<JsonArray>();
    }

    // check for duplicates
    for (JsonObject net : wifiList)
    {
        if (net["ssid"] == newSSID)
        {
            return true;
        }
    }

    // append the new network
    JsonObject added = wifiList.add<JsonObject>();
    added["ssid"] = newSSID;
    added["password"] = newPassword;

    // persist back to flash
    if (!storage.serialize(settingsDoc, settingsFilePath))
    {
        this->uart->println(F("[ERROR] Failed to write settings to storage."));
        return false;
    }

    this->uart->println(F("[SUCCESS] Settings saved."));
    return true;
}

void FlipperHTTP::setup()
{
    this->uart = new UART();
#ifdef BOARD_VGM
    this->uart->set_pins(0, 1);
#endif
    this->uart->begin(115200);
    this->uart->setTimeout(5000);
#if defined(BOARD_VGM)
    this->uart_2 = new UART();
    this->uart_2->set_pins(24, 21);
    this->uart_2->begin(115200);
    this->uart_2->setTimeout(5000);
    this->uart_2->flush();
#endif
    this->use_led = true;
    this->led.start();
    if (!storage.begin())
    {
        this->uart->println(F("[ERROR] Storage initialization failed."));
    }
    else
    {
        this->loadWiFi(); // Load WiFi settings
        String ledState = storage.read(ledStateFilePath);
        this->use_led = (ledState == "off") ? false : true;
    }
    this->uart->flush();
    this->led.off();
    this->http = new HTTP(this->uart, &this->client);
    this->websocket = nullptr;
}

// Main loop for flipper-http.ino that handles all of the commands
void FlipperHTTP::loop()
{
#ifdef BOARD_VGM
    // Check if there's incoming serial data
    if (this->uart->available() > 0)
    {
        if (this->use_led)
        {
            this->led.on();
        }

        // Read the incoming serial data until newline
        String _data = this->uart->readSerialLine();

        // send to ESP32
        this->uart_2->println(_data);

        // Wait for response from ESP32
        String _response = this->uart_2->readSerialLine();

        // Send response back to Flipper
        this->uart->println(_response);

        if (this->use_led)
        {
            this->led.off();
        }
    }
    else if (this->uart_2->available() > 0)
    {
        if (this->use_led)
        {
            this->led.on();
        }

        // Read the incoming serial data until newline
        String _data = this->uart_2->readSerialLine();

        // send to Flipper
        this->uart->println(_data);

        if (this->use_led)
        {
            this->led.off();
        }
    }
#else
    // Check if there's incoming serial data
    if (this->uart->available())
    {
        // Read the incoming serial data until newline
        String _data = this->uart->readSerialLine();
        CommandType commandType = commandFromString(_data);
        if (commandType == COMMAND_TYPE_UNKNOWN)
        {
            return;
        }

        if (this->use_led)
        {
            this->led.on();
        }

        switch (commandType)
        {
        case COMMAND_TYPE_LIST:
            this->uart->println(F("[LIST], [PING], [REBOOT], [WIFI/IP], [WIFI/SCAN], [WIFI/SAVE], [WIFI/CONNECT], [WIFI/DISCONNECT], [WIFI/LIST], [GET], [GET/HTTP], [POST/HTTP], [PUT/HTTP], [DELETE/HTTP], [GET/BYTES], [POST/BYTES], [PARSE], [PARSE/ARRAY], [LED/ON], [LED/OFF], [IP/ADDRESS], [WIFI/AP], [VERSION], [DEAUTH], [WIFI/STATUS], [WIFI/SSID], [BOARD/NAME]"));
            break;
        case COMMAND_TYPE_PING:
            this->uart->println("[PONG]");
            break;
        case COMMAND_TYPE_REBOOT:
            this->uart->println(F("Rebooting..."));
            commonReboot();
            break;
        case COMMAND_TYPE_WIFI_IP:
        {
            if (!this->wifi.isConnected() && !this->wifi.connect(loaded_ssid, loaded_pass))
            {
                this->uart->println(F("[ERROR] Not connected to Wifi. Failed to reconnect."));
                this->led.off();
                return;
            }
            // Get Request
            String jsonData = this->http->request("GET", "https://httpbin.org/get");
            if (jsonData == "")
            {
                this->uart->println(F("[ERROR] GET request failed or returned empty data."));
                return;
            }
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, jsonData);
            if (error)
            {
                this->uart->println(F("[ERROR] Failed to parse JSON."));
                this->led.off();
                return;
            }
            if (!doc["origin"])
            {
                this->uart->println(F("[ERROR] JSON does not contain origin."));
                this->led.off();
                return;
            }
            this->uart->println(doc["origin"].as<String>());
            this->uart->flush();
            this->uart->println();
            this->uart->println(F("[GET/END]"));
            break;
        }
        case COMMAND_TYPE_WIFI_SCAN:
        {
            this->uart->println(F("[GET/SUCCESS]"));
            this->uart->println(this->wifi.scan());
            this->uart->flush();
            this->uart->println();
            this->uart->println(F("[GET/END]"));
            break;
        }
        case COMMAND_TYPE_WIFI_SAVE:
        {
            // Extract JSON data by removing the command part
            String jsonData = _data.substring(strlen("[WIFI/SAVE]"));
            jsonData.trim(); // Remove any leading/trailing whitespace

            // Parse and save the settings
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, jsonData);

            if (error)
            {
                this->uart->print(F("[ERROR] Failed to parse JSON: "));
                this->uart->println(F(error.c_str()));
                return;
            }

            // Extract values from JSON
            if (doc["ssid"] && doc["password"])
            {
                strncpy(loaded_ssid, doc["ssid"], sizeof(loaded_ssid));     // save ssid
                strncpy(loaded_pass, doc["password"], sizeof(loaded_pass)); // save password
            }
            else
            {
                this->uart->println(F("[ERROR] JSON does not contain ssid and password."));
                return;
            }

            // Save to storage
            if (!this->saveWiFi(jsonData))
            {
                this->uart->println(F("[ERROR] Failed to save settings to file."));
                return;
            }

            if (this->wifi.isConnected())
            {
                this->wifi.disconnect();
            }

            // Attempt to reconnect with new settings
            if (this->wifi.connect(loaded_ssid, loaded_pass))
            {
                this->uart->println(F("[SUCCESS] WiFi settings saved and connected."));
                return;
            }
            else
            {
                this->uart->println(F("[ERROR] WiFi settings saved but failed to connect."));
                return;
            }

            this->uart->println(F("[SUCCESS] WiFi settings saved."));
            break;
        }
        case COMMAND_TYPE_WIFI_CONNECT:
        {
            // Check if WiFi is already connected
            if (!this->wifi.isConnected())
            {
                // Attempt to connect to Wifi
                if (this->wifi.connect(loaded_ssid, loaded_pass))
                {
                    this->uart->println(F("[SUCCESS] Connected to Wifi."));
                }
                else
                {
                    this->uart->println(F("[ERROR] Failed to connect to Wifi."));
                }
            }
            else
            {
                this->uart->println(F("[INFO] Already connected to WiFi."));
            }
            break;
        }
        case COMMAND_TYPE_WIFI_DISCONNECT:
        {
            this->wifi.disconnect();
            this->uart->println(F("[DISCONNECTED] WiFi has been disconnected."));
            break;
        }
        case COMMAND_TYPE_WIFI_LIST:
        {
            String fileContent = storage.read(settingsFilePath);
            this->uart->println(fileContent);
            this->uart->flush();
            break;
        }
        case COMMAND_TYPE_GET:
        {
            if (!this->wifi.isConnected() && !this->wifi.connect(loaded_ssid, loaded_pass))
            {
                this->uart->println(F("[ERROR] Not connected to WiFi. Failed to reconnect."));
                this->led.off();
                return;
            }
            // Extract URL by removing the command part
            String url = _data.substring(strlen("[GET]"));
            url.trim();

            // GET request
            String getData = this->http->request("GET", url);
            if (getData != "")
            {
                this->uart->println(getData);
                this->uart->flush();
                this->uart->println();
                this->uart->println(F("[GET/END]"));
            }
            else
            {
                this->uart->println(F("[ERROR] GET request failed or returned empty data."));
            }
            break;
        }
        case COMMAND_TYPE_GET_HTTP:
        {
            if (!this->wifi.isConnected() && !this->wifi.connect(loaded_ssid, loaded_pass))
            {
                this->uart->println(F("[ERROR] Not connected to Wifi. Failed to reconnect."));
                this->led.off();
                return;
            }

            // Extract the JSON by removing the command part
            String jsonData = _data.substring(strlen("[GET/HTTP]"));
            jsonData.trim();

            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, jsonData);

            if (error)
            {
                this->uart->print(F("[ERROR] Failed to parse JSON."));
                this->led.off();
                return;
            }

            // Extract values from JSON
            if (!doc["url"])
            {
                this->uart->println(F("[ERROR] JSON does not contain url."));
                this->led.off();
                return;
            }
            String url = doc["url"];

            // Extract headers if available
            const char *headerKeys[10];
            const char *headerValues[10];
            int headerSize = 0;

            if (doc["headers"])
            {
                JsonObject headers = doc["headers"];
                for (JsonPair header : headers)
                {
                    headerKeys[headerSize] = header.key().c_str();
                    headerValues[headerSize] = header.value();
                    headerSize++;
                }
            }

            // GET request
            String getData = this->http->request("GET", url, "", headerKeys, headerValues, headerSize);
            if (getData != "")
            {
                this->uart->println(getData);
                this->uart->flush();
                this->uart->println();
                this->uart->println(F("[GET/END]"));
            }
            else
            {
                this->uart->println(F("[ERROR] GET request failed or returned empty data."));
            }
            break;
        }
        case COMMAND_TYPE_POST_HTTP:
        {
            if (!this->wifi.isConnected() && !this->wifi.connect(loaded_ssid, loaded_pass))
            {
                this->uart->println(F("[ERROR] Not connected to Wifi. Failed to reconnect."));
                this->led.off();
                return;
            }

            // Extract the JSON by removing the command part
            String jsonData = _data.substring(strlen("[POST/HTTP]"));
            jsonData.trim();

            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, jsonData);

            if (error)
            {
                this->uart->print(F("[ERROR] Failed to parse JSON."));
                this->led.off();
                return;
            }

            // Extract values from JSON
            if (!doc["url"] || !doc["payload"])
            {
                this->uart->println(F("[ERROR] JSON does not contain url or payload."));
                this->led.off();
                return;
            }
            String url = doc["url"];
            String payload = doc["payload"];

            // Extract headers if available
            const char *headerKeys[10];
            const char *headerValues[10];
            int headerSize = 0;

            if (doc["headers"])
            {
                JsonObject headers = doc["headers"];
                for (JsonPair header : headers)
                {
                    headerKeys[headerSize] = header.key().c_str();
                    headerValues[headerSize] = header.value();
                    headerSize++;
                }
            }

            // POST request
            String postData = this->http->request("POST", url, payload, headerKeys, headerValues, headerSize);
            if (postData != "")
            {
                this->uart->println(postData);
                this->uart->flush();
                this->uart->println();
                this->uart->println(F("[POST/END]"));
            }
            else
            {
                this->uart->println(F("[ERROR] POST request failed or returned empty data."));
            }
            break;
        }
        case COMMAND_TYPE_PUT_HTTP:
        {
            if (!this->wifi.isConnected() && !this->wifi.connect(loaded_ssid, loaded_pass))
            {
                this->uart->println(F("[ERROR] Not connected to Wifi. Failed to reconnect."));
                this->led.off();
                return;
            }

            // Extract the JSON by removing the command part
            String jsonData = _data.substring(strlen("[PUT/HTTP]"));
            jsonData.trim();

            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, jsonData);

            if (error)
            {
                this->uart->print(F("[ERROR] Failed to parse JSON."));
                this->led.off();
                return;
            }

            // Extract values from JSON
            if (!doc["url"] || !doc["payload"])
            {
                this->uart->println(F("[ERROR] JSON does not contain url or payload."));
                this->led.off();
                return;
            }
            String url = doc["url"];
            String payload = doc["payload"];

            // Extract headers if available
            const char *headerKeys[10];
            const char *headerValues[10];
            int headerSize = 0;

            if (doc["headers"])
            {
                JsonObject headers = doc["headers"];
                for (JsonPair header : headers)
                {
                    headerKeys[headerSize] = header.key().c_str();
                    headerValues[headerSize] = header.value();
                    headerSize++;
                }
            }

            // PUT request
            String putData = this->http->request("PUT", url, payload, headerKeys, headerValues, headerSize);
            if (putData != "")
            {
                this->uart->println(putData);
                this->uart->flush();
                this->uart->println();
                this->uart->println(F("[PUT/END]"));
            }
            else
            {
                this->uart->println(F("[ERROR] PUT request failed or returned empty data."));
            }
            break;
        }
        case COMMAND_TYPE_DELETE_HTTP:
        {
            if (!this->wifi.isConnected() && !this->wifi.connect(loaded_ssid, loaded_pass))
            {
                this->uart->println(F("[ERROR] Not connected to Wifi. Failed to reconnect."));
                this->led.off();
                return;
            }

            // Extract the JSON by removing the command part
            String jsonData = _data.substring(strlen("[DELETE/HTTP]"));
            jsonData.trim();

            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, jsonData);

            if (error)
            {
                this->uart->print(F("[ERROR] Failed to parse JSON."));
                this->led.off();
                return;
            }

            // Extract values from JSON
            if (!doc["url"] || !doc["payload"])
            {
                this->uart->println(F("[ERROR] JSON does not contain url or payload."));
                this->led.off();
                return;
            }
            String url = doc["url"];
            String payload = doc["payload"];

            // Extract headers if available
            const char *headerKeys[10];
            const char *headerValues[10];
            int headerSize = 0;

            if (doc["headers"])
            {
                JsonObject headers = doc["headers"];
                for (JsonPair header : headers)
                {
                    headerKeys[headerSize] = header.key().c_str();
                    headerValues[headerSize] = header.value();
                    headerSize++;
                }
            }

            // DELETE request
            String deleteData = this->http->request("DELETE", url, payload, headerKeys, headerValues, headerSize);
            if (deleteData != "")
            {
                this->uart->println(deleteData);
                this->uart->flush();
                this->uart->println();
                this->uart->println(F("[DELETE/END]"));
            }
            else
            {
                this->uart->println(F("[ERROR] DELETE request failed or returned empty data."));
            }
            break;
        }
        case COMMAND_TYPE_GET_BYTES:
        {
            if (!this->wifi.isConnected() && !this->wifi.connect(loaded_ssid, loaded_pass))
            {
                this->uart->println(F("[ERROR] Not connected to Wifi. Failed to reconnect."));
                this->led.off();
                return;
            }

            // Extract the JSON by removing the command part
            String jsonData = _data.substring(strlen("[GET/BYTES]"));
            jsonData.trim();

            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, jsonData);

            if (error)
            {
                this->uart->print(F("[ERROR] Failed to parse JSON."));
                this->led.off();
                return;
            }

            // Extract values from JSON
            if (!doc["url"])
            {
                this->uart->println(F("[ERROR] JSON does not contain url."));
                this->led.off();
                return;
            }
            String url = doc["url"];

            // Extract headers if available
            const char *headerKeys[10];
            const char *headerValues[10];
            int headerSize = 0;

            if (doc["headers"])
            {
                JsonObject headers = doc["headers"];
                for (JsonPair header : headers)
                {
                    headerKeys[headerSize] = header.key().c_str();
                    headerValues[headerSize] = header.value();
                    headerSize++;
                }
            }

            // GET request
            if (!this->http->stream("GET", url, "", headerKeys, headerValues, headerSize))
            {
                this->uart->println(F("[ERROR] GET request failed or returned empty data."));
            }
            break;
        }
        case COMMAND_TYPE_POST_BYTES:
        {
            if (!this->wifi.isConnected() && !this->wifi.connect(loaded_ssid, loaded_pass))
            {
                this->uart->println(F("[ERROR] Not connected to Wifi. Failed to reconnect."));
                this->led.off();
                return;
            }

            // Extract the JSON by removing the command part
            String jsonData = _data.substring(strlen("[POST/BYTES]"));
            jsonData.trim();

            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, jsonData);

            if (error)
            {
                this->uart->print(F("[ERROR] Failed to parse JSON."));
                this->led.off();
                return;
            }

            // Extract values from JSON
            if (!doc["url"] || !doc["payload"])
            {
                this->uart->println(F("[ERROR] JSON does not contain url or payload."));
                this->led.off();
                return;
            }
            String url = doc["url"];
            String payload = doc["payload"];

            // Extract headers if available
            const char *headerKeys[10];
            const char *headerValues[10];
            int headerSize = 0;

            if (doc["headers"])
            {
                JsonObject headers = doc["headers"];
                for (JsonPair header : headers)
                {
                    headerKeys[headerSize] = header.key().c_str();
                    headerValues[headerSize] = header.value();
                    headerSize++;
                }
            }

            // POST request
            if (!this->http->stream("POST", url, payload, headerKeys, headerValues, headerSize))
            {
                this->uart->println(F("[ERROR] POST request failed or returned empty data."));
            }
            break;
        }
        case COMMAND_TYPE_PARSE:
        {
            // Extract the JSON by removing the command part
            String jsonData = _data.substring(strlen("[PARSE]"));
            jsonData.trim();

            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, jsonData);

            if (error)
            {
                this->uart->print(F("[ERROR] Failed to parse JSON."));
                this->led.off();
                return;
            }

            // Extract values from JSON
            if (!doc["key"] || !doc["json"])
            {
                this->uart->println(F("[ERROR] JSON does not contain key or json."));
                this->led.off();
                return;
            }
            String key = doc["key"];
            JsonObject json = doc["json"];

            if (json[key])
            {
                this->uart->println(json[key].as<String>());
            }
            else
            {
                this->uart->println(F("[ERROR] Key not found in JSON."));
            }
            break;
        }
        case COMMAND_TYPE_PARSE_ARRAY:
        {
            // Extract the JSON by removing the command part
            String jsonData = _data.substring(strlen("[PARSE/ARRAY]"));
            jsonData.trim();

            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, jsonData);

            if (error)
            {
                this->uart->print(F("[ERROR] Failed to parse JSON."));
                this->led.off();
                return;
            }

            // Extract values from JSON
            if (!doc["key"] || !doc["index"] || !doc["json"])
            {
                this->uart->println(F("[ERROR] JSON does not contain key, index, or json."));
                this->led.off();
                return;
            }
            String key = doc["key"];
            int index = doc["index"];
            JsonArray json = doc["json"];

            if (json[index][key])
            {
                this->uart->println(json[index][key].as<String>());
            }
            else
            {
                this->uart->println(F("[ERROR] Key not found in JSON."));
            }
            break;
        }
        case COMMAND_TYPE_LED_ON:
            this->use_led = true;
            if (storage.write(ledStateFilePath, "on"))
            {
                this->uart->println(F("[SUCCESS] LED enabled and state saved."));
            }
            else
            {
                this->uart->println(F("[ERROR] Failed to save LED state."));
            }
            break;
        case COMMAND_TYPE_LED_OFF:
            this->use_led = false;
            if (storage.write(ledStateFilePath, "off"))
            {
                this->uart->println(F("[SUCCESS] LED disabled and state saved."));
            }
            else
            {
                this->uart->println(F("[ERROR] Failed to save LED state."));
            }
            break;
        case COMMAND_TYPE_IP_ADDRESS:
            this->uart->println(this->wifi.deviceIP());
            break;
        case COMMAND_TYPE_WIFI_AP:
        {
            // Extract the JSON by removing the command part
            String jsonData = _data.substring(strlen("[WIFI/AP]"));
            jsonData.trim();

            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, jsonData);

            if (error)
            {
                this->uart->print(F("[ERROR] Failed to parse JSON."));
                this->led.off();
                return;
            }

            // Extract values from JSON
            if (!doc["ssid"])
            {
                this->uart->println(F("[ERROR] JSON does not contain ssid."));
                this->led.off();
                return;
            }

            String ssid = doc["ssid"];

            WiFiAP ap(this->uart, &this->wifi);

            if (!ap.start(ssid.c_str()))
            {
                this->led.off();
                return; // error is handled by class
            }

            this->uart->println(F("[AP/CONNECTED]"));
            ap.run();
            this->uart->println(F("[AP/DISCONNECTED]"));
            break;
        }
        case COMMAND_TYPE_VERSION:
            this->uart->println(FLIPPER_HTTP_VERSION);
            break;
        case COMMAND_TYPE_DEAUTH:
        {
            // Extract the JSON by removing the command part
            String jsonData = _data.substring(strlen("[DEAUTH]"));
            jsonData.trim();

            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, jsonData);

            if (error)
            {
                this->uart->print(F("[ERROR] Failed to parse JSON."));
                this->led.off();
                return;
            }

            // Extract values from JSON
            if (!doc["ssid"])
            {
                this->uart->println(F("[ERROR] JSON does not contain ssid"));
                this->led.off();
                return;
            }

            String ssid = doc["ssid"];

            WiFiDeauth deauther;
            this->uart->println(F("[DEAUTH/STARTING]"));

            if (!deauther.start(ssid.c_str()))
            {
                this->led.off();
                return; // error is handled by class
            }

            this->uart->println(F("[DEAUTH/STARTED]"));

            String uartMessage = "";
            while (uartMessage != "[DEAUTH/STOP]")
            {
                // Check if there's incoming serial data
                if (this->uart->available() > 0)
                {
                    // Read the incoming serial data until newline
                    uartMessage = this->uart->readSerialLine();
                }
                deauther.update();
            }
            deauther.stop();
            this->uart->println(F("[DEAUTH/STOPPED]"));
            break;
        }
        case COMMAND_TYPE_DEAUTH_STOP:
            // nothing to do..
            break;
        case COMMAND_TYPE_WIFI_STATUS:
            if (this->wifi.isConnected())
            {
                this->uart->println(F("true"));
            }
            else
            {
                this->uart->println(F("false"));
            }
            break;
        case COMMAND_TYPE_WIFI_SSID:
        {
            String ssid = this->wifi.getSSID();
            if (ssid != "")
            {
                this->uart->println(ssid);
            }
            else
            {
                this->uart->println(F("[ERROR] Not connected to WiFi."));
            }
            break;
        }
        case COMMAND_TYPE_BOARD_NAME:
            this->uart->println(commonGetBoardName());
            break;
        case COMMAND_TYPE_SOCKET_START:
        {
            // Remove the command prefix to isolate the JSON payload
            String jsonData = _data.substring(strlen("[SOCKET/START]"));
            jsonData.trim();

            // Create a JsonDocument with an appropriate size
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, jsonData);

            if (error)
            {
                this->uart->println(F("[ERROR] Failed to parse JSON."));
                this->led.off();
                return;
            }

            // Ensure that the JSON contains a "url" and "port"
            if (!doc["url"])
            {
                this->uart->println(F("[ERROR] JSON does not contain url."));
                this->led.off();
                return;
            }
            String fullUrl = doc["url"].as<String>();

            if (!doc["port"])
            {
                this->uart->println(F("[ERROR] JSON does not contain port."));
                this->led.off();
                return;
            }
            int port = doc["port"].as<int>();

            // Parse the fullUrl to extract server name and path.
            // Expected format: "ws://www.jblanked.com/ws/game/new/"
            String serverName;
            String path = "/";

            // Remove protocol ("ws://" or "wss://")
            if (fullUrl.startsWith("ws://"))
            {
                fullUrl = fullUrl.substring(5);
            }
            else if (fullUrl.startsWith("wss://"))
            {
                fullUrl = fullUrl.substring(6);
            }

            // Look for the first '/' that separates the server name from the path.
            int slashIndex = fullUrl.indexOf('/');
            if (slashIndex != -1)
            {
                serverName = fullUrl.substring(0, slashIndex);
                path = fullUrl.substring(slashIndex);
            }
            else
            {
                serverName = fullUrl;
                path = "/";
            }

            // Extract headers if available
            int headerSize = 0;
            const char *headerKeys[10];
            const char *headerValues[10];

            if (doc["headers"])
            {
                JsonObject headers = doc["headers"];
                for (JsonPair kv : headers)
                {
                    headerKeys[headerSize] = kv.key().c_str();
                    headerValues[headerSize] = kv.value().as<const char *>();
                    headerSize++;
                }
            }

            this->websocket = new WebSocket();

            if (!this->websocket || !this->websocket->connect(serverName.c_str(), port, path.c_str(), headerKeys, headerValues, headerSize))
            {
                this->uart->println(F("[ERROR] WebSocket connection failed."));
                this->led.off();
                return;
            }

            this->uart->println(F("[SOCKET/CONNECTED]"));

            // Check if a message is available from the server:
            String receivedMessage = this->websocket->recv();
            if (this->websocket->isConnected() && receivedMessage.length() > 0)
            {
                // Read the message from the server
                this->uart->println(receivedMessage);
            }

            // Wait for incoming serial/client data, and send back-n-forth
            String uartMessage = "";
            String wsMessage = "";
            while (this->websocket->isConnected() && !uartMessage.startsWith("[SOCKET/STOP]"))
            {
                // Check if there's incoming serial data
                if (this->uart->available() > 0)
                {
                    // Read the incoming serial data until newline
                    uartMessage = this->uart->readSerialLine();
                    this->websocket->send(uartMessage);
                    uartMessage = ""; // Clear the message after sending
                }

                // Check if there's incoming websocket data
                wsMessage = this->websocket->recv();
                if (wsMessage.length() > 0)
                {
                    // Read the message from the server
                    this->uart->println(wsMessage);
                    wsMessage = ""; // Clear the message after printing
                }
            }

            // Close the WebSocket connection
            this->websocket->stop();
            this->uart->println(F("[SOCKET/STOPPED]"));
            break;
        }
        case COMMAND_TYPE_SOCKET_STOP:
            // nothing to do..
            break;
        default:
            break;
        }

        if (this->use_led)
        {
            this->led.off();
        }
    }
#endif
}
