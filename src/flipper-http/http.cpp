#include "http.hpp"
#include "certs.hpp"
#include "common.hpp"
#include <ArduinoHttpClient.h>

#ifndef BOARD_BW16
HTTP::HTTP(UART *uart, WiFiClientSecure *client)
#else
HTTP::HTTP(UART *uart, WiFiSSLClient *client)
#endif
{
    this->uart = uart;
    this->client = client;

#ifndef BOARD_BW16
    this->client->setCACert(root_ca);
#else
    this->client->setRootCA((unsigned char *)root_ca);
#endif
}

String HTTP::request(
    const char *method,
    String url,
    String payload,
    const char *headerKeys[],
    const char *headerValues[],
    int headerSize)
#ifdef BOARD_BW16
{
    String response = "";                              // Initialize response string
    this->client->setRootCA((unsigned char *)root_ca); // Set root CA for SSL
    int index = url.indexOf('/');                      // Find the first occurrence of '/'
    String host = url.substring(0, index);             // Extract host
    String path = url.substring(index);                // Extract path

    char host_server[64];                                    // Buffer for host server
    strncpy(host_server, host.c_str(), sizeof(host_server)); // Copy host to buffer

    if (this->client->connect(host_server, 443)) // Connect to the server
    {
        // Make a HTTP request:
        this->client->print(method);
        this->client->print(" ");
        this->client->print(path);
        this->client->println(" HTTP/1.1");
        this->client->print("Host: ");
        this->client->println(host_server);

        // Add custom headers if provided
        for (int i = 0; i < headerSize; i++)
        {
            this->client->print(headerKeys[i]);
            this->client->print(": ");
            this->client->println(headerValues[i]);
        }

        // Add payload if provided
        if (payload != "")
        {
            this->client->print("Content-Length: ");
            this->client->println(payload.length());
            this->client->println("Content-Type: application/json");
        }

        this->client->println("Connection: close");
        this->client->println();

        // Send the payload in the request body
        if (payload != "")
        {
            this->client->println(payload);
        }

        // read everything that’s in the buffer, then stop
        while (this->client->available())
            response += this->client->readStringUntil('\n') + "\n";
        this->client->stop();
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
{
    HTTPClient http;
    String response = "";

    http.collectHeaders(headerKeys, headerSize);

    if (http.begin(*this->client, url))
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
                this->client->setInsecure();
                if (http.begin(*this->client, url))
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
                        this->client->setCACert(root_ca);
                        return response;
                    }
                    else
                    {
                        this->client->setCACert(root_ca);
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

bool HTTP::stream(const char *method, String url, String payload, const char *headerKeys[], const char *headerValues[], int headerSize)
#ifdef BOARD_BW16
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
{
    HTTPClient http;

    http.collectHeaders(headerKeys, headerSize);

    if (http.begin(*this->client, url))
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

            size_t freeHeap = commonGetFreeHeap(); // Check available heap memory before starting
            const size_t minHeapThreshold = 1024;  // Minimum heap space to avoid overflow
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
            freeHeap = commonGetFreeHeap(); // Check available heap memory after processing
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
                this->client->setInsecure();
                if (http.begin(*this->client, url))
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
                        size_t freeHeap = commonGetFreeHeap();
                        if (freeHeap < 1024)
                        {
                            this->uart->println(F("[ERROR] Not enough memory to start processing the response."));
                            http.end();
                            this->client->setCACert(root_ca);
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

                        freeHeap = commonGetFreeHeap(); // Check available heap memory after processing
                        if (freeHeap < 1024)
                        {
                            this->uart->println(F("[ERROR] Not enough memory to continue processing the response."));
                            http.end();
                            this->client->setCACert(root_ca);
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
                        this->client->setCACert(root_ca);
                        return true;
                    }
                    else
                    {
                        this->client->setCACert(root_ca);
                        snprintf(headerResponse, sizeof(headerResponse), "[ERROR] %s Request Failed, error: %s", method, http.errorToString(newCode).c_str());
                        this->uart->println(headerResponse);
                    }
                }
                this->client->setCACert(root_ca);
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

bool HTTP::streamUpload(const char *method, String url, size_t fileSize, String contentType, const char *headerKeys[], const char *headerValues[], int headerSize)
#ifdef BOARD_BW16
{
    this->uart->println(F("[ERROR] streamUpload not implemented for BW16."));
    return false;
}
#else
{
    // Parse URL into host and path
    String host, path;
    int port = 443;

    if (url.startsWith("https://"))
    {
        url.remove(0, 8);
        port = 443;
    }
    else if (url.startsWith("http://"))
    {
        url.remove(0, 7);
        port = 80;
    }

    int slashIdx = url.indexOf('/');
    if (slashIdx != -1)
    {
        host = url.substring(0, slashIdx);
        path = url.substring(slashIdx);
    }
    else
    {
        host = url;
        path = "/";
    }

    // Connect to the server before signalling ready, so the device
    // doesn't start sending bytes to an unconnected upload.
    if (!this->client->connect(host.c_str(), port))
    {
        this->client->setInsecure();
        if (!this->client->connect(host.c_str(), port))
        {
            this->uart->println(F("[ERROR] Failed to connect to server for upload."));
            this->client->setCACert(root_ca);
            return false;
        }
    }

    // Send HTTP request line and headers
    this->client->print(method);
    this->client->print(F(" "));
    this->client->print(path);
    this->client->println(F(" HTTP/1.1"));
    this->client->print(F("Host: "));
    this->client->println(host);
    this->client->print(F("Content-Type: "));
    this->client->println(contentType);
    this->client->print(F("Content-Length: "));
    this->client->println(fileSize);
    for (int i = 0; i < headerSize; i++)
    {
        this->client->print(headerKeys[i]);
        this->client->print(F(": "));
        this->client->println(headerValues[i]);
    }
    this->client->println(F("Connection: close"));
    this->client->println(); // blank line ends headers

    // Signal the UART device that we are ready for raw bytes
    this->uart->println(F("[FILE/READY]"));
    this->uart->flush();

    // Stream body: read chunks from UART and write directly to the HTTP connection
    uint8_t buf[128];
    size_t remaining = fileSize;
    unsigned long timeoutStart = millis();
    const unsigned long chunkTimeout = 5000; // 5 s without new data = abort

    while (remaining > 0)
    {
        size_t avail = this->uart->available();
        if (avail > 0)
        {
            size_t toRead = avail < remaining ? avail : remaining;
            if (toRead > sizeof(buf))
                toRead = sizeof(buf);
            size_t bytesRead = this->uart->readBytes(buf, (uint8_t)toRead);
            this->client->write(buf, bytesRead);
            remaining -= bytesRead;
            timeoutStart = millis();
        }
        else
        {
            if (millis() - timeoutStart > chunkTimeout)
            {
                this->uart->println(F("[ERROR] Upload timed out waiting for data."));
                this->client->stop();
                this->client->setCACert(root_ca);
                return false;
            }
            delay(1);
        }
    }

    // Wait for the server's response headers to arrive
    unsigned long responseTimeout = millis();
    while (!this->client->available() && millis() - responseTimeout < 5000)
    {
        delay(1);
    }

    // Parse HTTP status line: "HTTP/1.1 200 OK\r\n"
    int statusCode = 0;
    String statusLine = this->client->readStringUntil('\n');
    statusLine.trim();
    int sp = statusLine.indexOf(' ');
    if (sp != -1)
    {
        statusCode = statusLine.substring(sp + 1, sp + 4).toInt();
    }

    // Parse response headers: capture Content-Length, stop at blank line
    int contentLength = -1;
    while (this->client->available())
    {
        String headerLine = this->client->readStringUntil('\n');
        headerLine.trim();
        if (headerLine.length() == 0)
            break;
        if (headerLine.startsWith("Content-Length:") || headerLine.startsWith("content-length:"))
        {
            contentLength = headerLine.substring(headerLine.indexOf(':') + 1).toInt();
        }
    }

    // Stream response body back over UART in chunks
    uint8_t rbuf[512] = {0};
    char headerResponse[128];
    snprintf(headerResponse, sizeof(headerResponse),
             "[POST/SUCCESS]{\"Status-Code\":%d,\"Content-Length\":%d}", statusCode, contentLength);
    this->uart->println(headerResponse);

    unsigned long bodyTimeout = millis();
    while (this->client->connected())
    {
        size_t size = this->client->available();
        if (size)
        {
            bodyTimeout = millis();
            int c = this->client->readBytes(rbuf, size > sizeof(rbuf) ? sizeof(rbuf) : size);
            this->uart->write(rbuf, c);
        }
        else
        {
            if (millis() - bodyTimeout > 2000)
                break;
            delay(1);
        }
    }

    this->client->stop();
    this->client->setCACert(root_ca);
    this->uart->flush();
    this->uart->println();
    this->uart->println(F("[POST/END]"));
    return true;
}
#endif