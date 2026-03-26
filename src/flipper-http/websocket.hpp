#pragma once
#include <Arduino.h>
#include <ArduinoHttpClient.h>

class WebSocket
{
public:
    WebSocket();
    ~WebSocket();

    bool connect(
        const char *serverName,               // The server name or IP address to connect to (e.g. "example.com"
        uint16_t port,                        // The port to connect to on the server (e.g. 80 for ws:// and 443 for wss://)
        const char *path,                     // The path to connect to on the server (e.g. "/ws")
        const char *headerKeys[] = nullptr,   // Array of header keys
        const char *headerValues[] = nullptr, // Array of header values
        int headerSize = 0                    // Number of headers
    );
    bool isConnected();
    void ping();
    String recv();
    void send(String &message);
    void stop();

private:
    WebSocketClient *ws_client;
};