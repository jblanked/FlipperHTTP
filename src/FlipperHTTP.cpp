/* FlipperHTTP.cpp for flipper-http.ino and FlipperHTTP.h
Author: JBlanked
Github: https://github.com/jblanked/FlipperHTTP
Info: This library is a wrapper around the HTTPClient library and is used to communicate with the FlipperZero over serial.
Created: 2024-09-30
Updated: 2025-12-22
*/

#include "FlipperHTTP.hpp"
#include "wifi_ap.hpp"
#include "wifi_deauth.hpp"

#define MAX_CHUNK_SIZE 128

// Load WiFi settings
bool FlipperHTTP::loadWiFi()
{
    JsonDocument doc;
    if (!storage.deserialize(doc, settingsFilePath))
    {
        this->uart->println(F("[ERROR] Failed to deserialize JSON from settings file."));
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

#ifdef BOARD_BW16
String FlipperHTTP::request(
    const char *method,
    String url,
    String payload,
    const char *headerKeys[],
    const char *headerValues[],
    int headerSize)
{
    String response = "";                             // Initialize response string
    this->client.setRootCA((unsigned char *)root_ca); // Set root CA for SSL
    int index = url.indexOf('/');                     // Find the first occurrence of '/'
    String host = url.substring(0, index);            // Extract host
    String path = url.substring(index);               // Extract path

    char host_server[64];                                    // Buffer for host server
    strncpy(host_server, host.c_str(), sizeof(host_server)); // Copy host to buffer

    if (this->client.connect(host_server, 443)) // Connect to the server
    {
        // Make a HTTP request:
        this->client.print(method);
        this->client.print(" ");
        this->client.print(path);
        this->client.println(" HTTP/1.1");
        this->client.print("Host: ");
        this->client.println(host_server);

        // Add custom headers if provided
        for (int i = 0; i < headerSize; i++)
        {
            this->client.print(headerKeys[i]);
            this->client.print(": ");
            this->client.println(headerValues[i]);
        }

        // Add payload if provided
        if (payload != "")
        {
            this->client.print("Content-Length: ");
            this->client.println(payload.length());
            this->client.println("Content-Type: application/json");
        }

        this->client.println("Connection: close");
        this->client.println();

        // Send the payload in the request body
        if (payload != "")
        {
            this->client.println(payload);
        }

        // read everything that’s in the buffer, then stop
        while (this->client.available())
            response += this->client.readStringUntil('\n') + "\n";
        this->client.stop();
    }
    else
    {
        this->uart->println(F("[ERROR] Unable to connect to the server."));
    }

    // Clear serial buffer to avoid any residual data
    this->uart->clearBuffer();

    return response;
}
#else
String FlipperHTTP::request(
    const char *method,
    String url,
    String payload,
    const char *headerKeys[],
    const char *headerValues[],
    int headerSize)
{
    HTTPClient http;
    String response = "";

    http.collectHeaders(headerKeys, headerSize);

    if (http.begin(this->client, url))
    {
        for (int i = 0; i < headerSize; i++)
        {
            http.addHeader(headerKeys[i], headerValues[i]);
        }

        if (payload == "")
        {
            payload = "{}";
        }

        int statusCode = http.sendRequest(method, payload);
        char headerResponse[512];

        if (statusCode > 0)
        {
            snprintf(headerResponse, sizeof(headerResponse), "[%s/SUCCESS]{\"Status-Code\":%d,\"Content-Length\":%d}", method, statusCode, http.getSize());
            this->uart->println(headerResponse);
            response = http.getString();
            http.end();
            return response;
        }
        else
        {
            if (statusCode != -1) // HTTPC_ERROR_CONNECTION_FAILED
            {
                snprintf(headerResponse, sizeof(headerResponse), "[ERROR] %s Request Failed, error: %s", method, http.errorToString(statusCode).c_str());
                this->uart->println(headerResponse);
            }
            else // certification failed?
            {
                // send request without SSL
                http.end();
                this->client.setInsecure();
                if (http.begin(this->client, url))
                {
                    for (int i = 0; i < headerSize; i++)
                    {
                        http.addHeader(headerKeys[i], headerValues[i]);
                    }
                    int newCode = http.sendRequest(method, payload);
                    if (newCode > 0)
                    {
                        snprintf(headerResponse, sizeof(headerResponse), "[%s/SUCCESS]{\"Status-Code\":%d,\"Content-Length\":%d}", method, newCode, http.getSize());
                        this->uart->println(headerResponse);
                        response = http.getString();
                        http.end();
                        this->client.setCACert(root_ca);
                        return response;
                    }
                    else
                    {
                        this->client.setCACert(root_ca);
                        snprintf(headerResponse, sizeof(headerResponse), "[ERROR] %s Request Failed, error: %s", method, http.errorToString(newCode).c_str());
                        this->uart->println(headerResponse);
                    }
                }
            }
        }
        http.end();
    }
    else
    {
        this->uart->println(F("[ERROR] Unable to connect to the server."));
    }

    // Clear serial buffer to avoid any residual data
    this->uart->clearBuffer();

    return response;
}
#endif

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

void FlipperHTTP::sendLargeMessage(WebSocketClient &ws, String message)
{
    int totalLength = message.length();

    if (totalLength <= MAX_CHUNK_SIZE)
    {
        // If message is small enough, send normally
        ws.beginMessage(TYPE_TEXT);
        ws.print(message);
        ws.endMessage();
        return;
    }

    // Send in chunks
    for (int i = 0; i < totalLength; i += MAX_CHUNK_SIZE)
    {
        String chunk = message.substring(i, min(i + MAX_CHUNK_SIZE, totalLength));
        ws.beginMessage(TYPE_TEXT);
        ws.print(chunk);
        ws.endMessage();
        delay(10); // Small delay between chunks
    }
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
    }
#ifndef BOARD_BW16
    this->client.setCACert(root_ca);
#else
    this->client.setRootCA((unsigned char *)root_ca);
#endif
    this->uart->flush();
    this->led.off();
}

#ifdef BOARD_BW16
bool FlipperHTTP::streamBytes(const char *method, String url, String payload, const char *headerKeys[], const char *headerValues[], int headerSize)
{
    // Not implemented for BW16
    this->uart->print(F("[ERROR] streamBytes not implemented for BW16."));
    this->uart->print(method);
    this->uart->print(url);
    this->uart->print(payload);
    for (int i = 0; i < headerSize; i++)
    {
        this->uart->print(headerKeys[i]);
        this->uart->print(headerValues[i]);
    }
    this->uart->println();
    return false;
}
#else
bool FlipperHTTP::streamBytes(const char *method, String url, String payload, const char *headerKeys[], const char *headerValues[], int headerSize)
{
    HTTPClient http;

    http.collectHeaders(headerKeys, headerSize);

    if (http.begin(this->client, url))
    {
        for (int i = 0; i < headerSize; i++)
        {
            http.addHeader(headerKeys[i], headerValues[i]);
        }

        if (payload == "")
        {
            payload = "{}";
        }

        int httpCode = http.sendRequest(method, payload);
        int len = http.getSize(); // Get the response content length
        char headerResponse[256];
        if (httpCode > 0)
        {
            snprintf(headerResponse, sizeof(headerResponse), "[%s/SUCCESS]{\"Status-Code\":%d,\"Content-Length\":%d}", method, httpCode, len);
            this->uart->println(headerResponse);
            uint8_t buff[512] = {0}; // Buffer for reading data

            WiFiClient *stream = http.getStreamPtr();

            size_t freeHeap = storage.freeHeap(); // Check available heap memory before starting
            const size_t minHeapThreshold = 1024; // Minimum heap space to avoid overflow
            if (freeHeap < minHeapThreshold)
            {
                this->uart->println(F("[ERROR] Not enough memory to start processing the response."));
                http.end();
                return false;
            }

            // Start timeout timer
            unsigned long timeoutStart = millis();
            const unsigned long timeoutInterval = 2000; // 2 seconds

            // Stream data while connected and available
            while (http.connected() && (len > 0 || len == -1))
            {
                size_t size = stream->available();
                if (size)
                {
                    // Reset the timeout when new data comes in
                    timeoutStart = millis();

                    int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
                    this->uart->write(buff, c); // Write data to serial
                    if (len > 0)
                    {
                        len -= c;
                    }
                }
                else
                {
                    // Check if timeout has been reached
                    if (millis() - timeoutStart > timeoutInterval)
                    {
                        break;
                    }
                }
                delay(1); // Yield control to the system
            }
            freeHeap = storage.freeHeap(); // Check available heap memory after processing
            if (freeHeap < minHeapThreshold)
            {
                this->uart->println(F("[ERROR] Not enough memory to continue processing the response."));
                http.end();
                return false;
            }

            http.end();
            // Flush the serial buffer to ensure all data is sent
            this->uart->flush();
            this->uart->println();
            if (strcmp(method, "GET") == 0)
            {
                this->uart->println(F("[GET/END]"));
            }
            else
            {
                this->uart->println(F("[POST/END]"));
            }
            return true;
        }
        else
        {
            if (httpCode != -1) // HTTPC_ERROR_CONNECTION_FAILED
            {
                snprintf(headerResponse, sizeof(headerResponse), "[ERROR] %s Request Failed, error: %s", method, http.errorToString(httpCode).c_str());
                this->uart->println(headerResponse);
            }
            else // certification failed?
            {
                // Send request without SSL
                http.end();
                this->client.setInsecure();
                if (http.begin(this->client, url))
                {
                    for (int i = 0; i < headerSize; i++)
                    {
                        http.addHeader(headerKeys[i], headerValues[i]);
                    }
                    int newCode = http.sendRequest(method, payload);
                    int len = http.getSize(); // Get the response content length
                    if (newCode > 0)
                    {
                        snprintf(headerResponse, sizeof(headerResponse), "[%s/SUCCESS]{\"Status-Code\":%d,\"Content-Length\":%d}", method, newCode, len);
                        this->uart->println(headerResponse);
                        uint8_t buff[512] = {0}; // Buffer for reading data

                        WiFiClient *stream = http.getStreamPtr();

                        // Check available heap memory before starting
                        size_t freeHeap = storage.freeHeap();
                        if (freeHeap < 1024)
                        {
                            this->uart->println(F("[ERROR] Not enough memory to start processing the response."));
                            http.end();
                            this->client.setCACert(root_ca);
                            return false;
                        }

                        // Start timeout timer
                        unsigned long timeoutStart = millis();
                        const unsigned long timeoutInterval = 2000; // 2 seconds

                        // Stream data while connected and available
                        while (http.connected() && (len > 0 || len == -1))
                        {
                            size_t size = stream->available();
                            if (size)
                            {
                                // Reset the timeout when new data arrives
                                timeoutStart = millis();

                                int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
                                this->uart->write(buff, c); // Write data to serial
                                if (len > 0)
                                {
                                    len -= c;
                                }
                            }
                            else
                            {
                                // Check if timeout has been reached
                                if (millis() - timeoutStart > timeoutInterval)
                                {
                                    break;
                                }
                            }
                            delay(1); // Yield control to the system
                        }

                        freeHeap = storage.freeHeap(); // Check available heap memory after processing
                        if (freeHeap < 1024)
                        {
                            this->uart->println(F("[ERROR] Not enough memory to continue processing the response."));
                            http.end();
                            this->client.setCACert(root_ca);
                            return false;
                        }

                        http.end();
                        // Flush the serial buffer to ensure all data is sent
                        this->uart->flush();
                        this->uart->println();
                        if (strcmp(method, "GET") == 0)
                        {
                            this->uart->println(F("[GET/END]"));
                        }
                        else
                        {
                            this->uart->println(F("[POST/END]"));
                        }
                        this->client.setCACert(root_ca);
                        return true;
                    }
                    else
                    {
                        this->client.setCACert(root_ca);
                        snprintf(headerResponse, sizeof(headerResponse), "[ERROR] %s Request Failed, error: %s", method, http.errorToString(newCode).c_str());
                        this->uart->println(headerResponse);
                    }
                }
                this->client.setCACert(root_ca);
            }
        }
        http.end();
    }
    else
    {
        this->uart->println(F("[ERROR] Unable to connect to the server."));
    }
    return false;
}
#endif

bool FlipperHTTP::readSerialSettings(String receivedData, bool connectAfterSave)
{
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, receivedData);

    if (error)
    {
        this->uart->print(F("[ERROR] Failed to parse JSON: "));
        this->uart->println(F(error.c_str()));
        return false;
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
        return false;
    }

    // Save to storage
    if (!this->saveWiFi(receivedData))
    {
        this->uart->println(F("[ERROR] Failed to save settings to file."));
        return false;
    }

    // Attempt to reconnect with new settings
    if (connectAfterSave && this->wifi.connect(loaded_ssid, loaded_pass))
    {
        this->uart->println(F("[SUCCESS] Connected to the new Wifi network."));
    }

    return true;
}

// Main loop for flipper-http.ino that handles all of the commands
void FlipperHTTP::loop()
{
#ifdef BOARD_VGM
    // Check if there's incoming serial data
    if (this->uart->available() > 0)
    {
        this->led.on();

        // Read the incoming serial data until newline
        String _data = this->uart->readSerialLine();

        // send to ESP32
        this->uart_2->println(_data);

        // Wait for response from ESP32
        String _response = this->uart_2->readSerialLine();

        // Send response back to Flipper
        this->uart->println(_response);

        this->led.off();
    }
    else if (this->uart_2->available() > 0)
    {
        this->led.on();

        // Read the incoming serial data until newline
        String _data = this->uart_2->readSerialLine();

        // send to Flipper
        this->uart->println(_data);

        this->led.off();
    }
#else
    // Check if there's incoming serial data
    if (this->uart->available())
    {
        // Read the incoming serial data until newline
        String _data = this->uart->readSerialLine();

        if (_data.length() == 0)
        {
            // No complete command received
            return;
        }

        this->led.on();

        // print the available commands
        if (_data.startsWith("[LIST]"))
        {
            this->uart->println(F("[LIST], [PING], [REBOOT], [WIFI/IP], [WIFI/SCAN], [WIFI/SAVE], [WIFI/CONNECT], [WIFI/DISCONNECT], [WIFI/LIST], [GET], [GET/HTTP], [POST/HTTP], [PUT/HTTP], [DELETE/HTTP], [GET/BYTES], [POST/BYTES], [PARSE], [PARSE/ARRAY], [LED/ON], [LED/OFF], [IP/ADDRESS], [WIFI/AP], [VERSION], [DEAUTH], [WIFI/STATUS]"));
        }
        // handle [LED/ON] command
        else if (_data.startsWith("[LED/ON]"))
        {
            this->use_led = true;
        }
        // handle [LED/OFF] command
        else if (_data.startsWith("[LED/OFF]"))
        {
            this->use_led = false;
        }
        // handle [VERSION] command
        else if (_data.startsWith("[VERSION]"))
        {
            this->uart->println(FLIPPER_HTTP_VERSION);
        }
        // handle [IP/ADDRESS] command (local IP)
        else if (_data.startsWith("[IP/ADDRESS]"))
        {
            this->uart->println(this->wifi.deviceIP());
        }
        // handle [WIFI/IP] command ip of connected wifi
        else if (_data.startsWith("[WIFI/IP]"))
        {
            if (!this->wifi.isConnected() && !this->wifi.connect(loaded_ssid, loaded_pass))
            {
                this->uart->println(F("[ERROR] Not connected to Wifi. Failed to reconnect."));
                this->led.off();
                return;
            }
            // Get Request
            String jsonData = this->request("GET", "https://httpbin.org/get");
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
        }
        // Ping/Pong to see if board/flipper is connected
        else if (_data.startsWith("[PING]"))
        {
            this->uart->println("[PONG]");
        }
        // Handle [REBOOT] command
        else if (_data.startsWith("[REBOOT]"))
        {
            this->use_led = true;
#if defined(BOARD_PICO_W) || defined(BOARD_PICO_2W) || defined(BOARD_VGM) || defined(BOARD_PICOCALC_W) || defined(BOARD_PICOCALC_2W)
            rp2040.reboot();
#elif defined(BOARD_BW16)
            ota_platform_reset();
#else
            ESP.restart();
#endif
        }
        // scan for wifi networks
        else if (_data.startsWith("[WIFI/SCAN]"))
        {
            this->uart->println(F("[GET/SUCCESS]"));
            this->uart->println(this->wifi.scan());
            this->uart->flush();
            this->uart->println();
            this->uart->println(F("[GET/END]"));
        }
        // Handle Wifi list command
        else if (_data.startsWith("[WIFI/LIST]"))
        {
            String fileContent = storage.read(settingsFilePath);
            this->uart->println(fileContent);
            this->uart->flush();
        }
        // Handle [WIFI/SAVE] command
        else if (_data.startsWith("[WIFI/SAVE]"))
        {
            // Extract JSON data by removing the command part
            String jsonData = _data.substring(strlen("[WIFI/SAVE]"));
            jsonData.trim(); // Remove any leading/trailing whitespace

            // Parse and save the settings
            if (this->readSerialSettings(jsonData, true))
            {
                this->uart->println(F("[SUCCESS] Wifi settings saved."));
            }
            else
            {
                this->uart->println(F("[ERROR] Failed to save Wifi settings."));
            }
        }
        // Handle [WIFI/STATUS] command
        else if (_data == "[WIFI/STATUS]")
        {
            if (this->wifi.isConnected())
            {
                this->uart->println(F("true"));
            }
            else
            {
                this->uart->println(F("false"));
            }
        }
        // Handle [WIFI/SSID] command
        else if (_data == "[WIFI/SSID]")
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
        }
        // Handle [WIFI/CONNECT] command
        else if (_data == "[WIFI/CONNECT]")
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
        }
        // Handle [WIFI/DISCONNECT] command
        else if (_data == "[WIFI/DISCONNECT]")
        {
            this->wifi.disconnect();
            this->uart->println(F("[DISCONNECTED] WiFi has been disconnected."));
        }
        // Handle [GET] command
        else if (_data.startsWith("[GET]"))
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
            String getData = this->request("GET", url);
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
        }
        // Handle [GET/HTTP] command
        else if (_data.startsWith("[GET/HTTP]"))
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
            String getData = this->request("GET", url, "", headerKeys, headerValues, headerSize);
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
        }
        // Handle [POST/HTTP] command
        else if (_data.startsWith("[POST/HTTP]"))
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
            String postData = this->request("POST", url, payload, headerKeys, headerValues, headerSize);
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
        }
        // Handle [PUT/HTTP] command
        else if (_data.startsWith("[PUT/HTTP]"))
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
            String putData = this->request("PUT", url, payload, headerKeys, headerValues, headerSize);
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
        }
        // Handle [DELETE/HTTP] command
        else if (_data.startsWith("[DELETE/HTTP]"))
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
            String deleteData = this->request("DELETE", url, payload, headerKeys, headerValues, headerSize);
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
        }

        // Handle [GET/BYTES]
        else if (_data.startsWith("[GET/BYTES]"))
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
            if (!this->streamBytes("GET", url, "", headerKeys, headerValues, headerSize))
            {
                this->uart->println(F("[ERROR] GET request failed or returned empty data."));
            }
        }
        // handle [POST/BYTES]
        else if (_data.startsWith("[POST/BYTES]"))
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
            if (!this->streamBytes("POST", url, payload, headerKeys, headerValues, headerSize))
            {
                this->uart->println(F("[ERROR] POST request failed or returned empty data."));
            }
        }
        // Handle [PARSE] command
        else if (_data.startsWith("[PARSE]"))
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
        }
        // Handle [PARSE/ARRAY] command
        else if (_data.startsWith("[PARSE/ARRAY]"))
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
        }
        // websocket
        else if (_data.startsWith("[SOCKET/START]"))
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

            /*
              weirdly enough, using our client didn't work for all websites
              in the future we should try to connect with our client first,
              then if it fails, use the WiFiClient.
            */
            WiFiClient wifi_client;

            // Create your WebSocketClient with the client, server name, and port
            WebSocketClient ws = WebSocketClient(wifi_client, serverName.c_str(), port);

            // Send headers, if any
            for (int i = 0; i < headerSize; i++)
            {
                ws.sendHeader(headerKeys[i], headerValues[i]);
            }

            // Begin the WebSocket connection, passing in the extracted path
            ws.begin(path.c_str());

            if (!ws.connected())
            {
                this->uart->println(F("[ERROR] WebSocket connection failed."));
                this->led.off();
                return;
            }

            this->uart->println(F("[SOCKET/CONNECTED]"));

            // Check if a message is available from the server:
            if (ws.parseMessage() > 0)
            {
                // Read the message from the server
                String message = ws.readString();
                this->uart->println(message);
            }

            // Wait for incoming serial/client data, and send back-n-forth
            String uartMessage = "";
            String wsMessage = "";
            while (ws.connected() && !uartMessage.startsWith("[SOCKET/STOP]"))
            {
                // Check if there's incoming serial data
                if (this->uart->available() > 0)
                {
                    // Read the incoming serial data until newline
                    uartMessage = this->uart->readSerialLine();
                    sendLargeMessage(ws, uartMessage);
                }

                // Check if there's incoming websocket data
                if (ws.parseMessage() > 0)
                {
                    // Read the message from the server
                    wsMessage = ws.readString();
                    this->uart->println(wsMessage);
                }
            }

            // Close the WebSocket connection
            ws.stop();
            this->uart->println(F("[SOCKET/STOPPED]"));
        }
        // [WIFI/AP] AP Mode
        else if (_data.startsWith("[WIFI/AP]"))
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
        }
        // [DEAUTH] Deauth command
        else if (_data.startsWith("[DEAUTH]"))
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
        }

        this->led.off();
    }
#endif
}
