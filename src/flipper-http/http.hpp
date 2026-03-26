#pragma once
#include <Arduino.h>
#include "wifi_utils.hpp"
#include "uart.hpp"
#include "boards.hpp"

class HTTP
{

public:
#ifndef BOARD_BW16
    HTTP(UART *uart, WiFiClientSecure *client);
#else
    HTTP(UART *uart, WiFiSSLClient *client);
#endif
    ~HTTP() {} // Destructor

    // returns the response as a string, or an empty string if the request failed
    String request(
        const char *method,                   // HTTP method
        String url,                           // URL to send the request to
        String payload = "",                  // Payload to send with the request
        const char *headerKeys[] = nullptr,   // Array of header keys
        const char *headerValues[] = nullptr, // Array of header values
        int headerSize = 0                    // Number of headers
    );

    // Streams the response in chunks over UART, returns false if the request failed
    bool stream(const char *method, String url, String payload, const char *headerKeys[], const char *headerValues[], int headerSize);

    // Reads fileSize raw bytes from UART and uploads them as the request body,
    // then streams the response back over UART. Returns false on failure.
    bool streamUpload(const char *method, String url, size_t fileSize, String contentType, const char *headerKeys[], const char *headerValues[], int headerSize);

private:
#ifndef BOARD_BW16
    WiFiClientSecure *client; // WiFiClientSecure object for secure connections
#else
    WiFiSSLClient *client; // WiFiSSLClient object for secure connections
#endif
    UART *uart; // UART object to handle serial communication
};
