#include "websocket.hpp"
#include "wifi_utils.hpp"

#define MAX_WS_CHUNK_SIZE 128

WebSocket::WebSocket()
{
    this->ws_client = nullptr;
}

WebSocket::~WebSocket()
{
    if (this->ws_client)
    {
        this->ws_client->stop();
        delete this->ws_client;
        this->ws_client = nullptr;
    }
}

bool WebSocket::connect(
    const char *serverName,
    uint16_t port,
    const char *path,
    const char *headerKeys[],
    const char *headerValues[],
    int headerSize)
{
    WiFiClient wifi_client;
    this->ws_client = new WebSocketClient(wifi_client, serverName, port);

    // Send headers, if any
    for (int i = 0; i < headerSize; i++)
    {
        this->ws_client->sendHeader(headerKeys[i], headerValues[i]);
    }

    // Begin the WebSocket connection
    this->ws_client->begin(path);

    return this->ws_client->connected();
}

bool WebSocket::isConnected()
{
    return this->ws_client && this->ws_client->connected();
}

void WebSocket::ping()
{
    if (this->ws_client && this->ws_client->connected())
    {
        this->ws_client->ping();
    }
}

String WebSocket::recv()
{
    if (this->ws_client && this->ws_client->connected())
    {
        if (this->ws_client->parseMessage() > 0)
        {
            return this->ws_client->readString();
        }
    }
    return "";
}

void WebSocket::send(String &message)
{
    if (this->ws_client && this->ws_client->connected())
    {
        int totalLength = message.length();

        if (totalLength <= MAX_WS_CHUNK_SIZE)
        {
            // If message is small enough, send normally
            this->ws_client->beginMessage(TYPE_TEXT);
            this->ws_client->print(message);
            this->ws_client->endMessage();
            return;
        }

        // Send in chunks
        for (int i = 0; i < totalLength; i += MAX_WS_CHUNK_SIZE)
        {
            String chunk = message.substring(i, min(i + MAX_WS_CHUNK_SIZE, totalLength));
            this->ws_client->beginMessage(TYPE_TEXT);
            this->ws_client->print(chunk);
            this->ws_client->endMessage();
            delay(10); // Small delay between chunks
        }
    }
}

void WebSocket::stop()
{
    if (this->ws_client)
    {
        this->ws_client->stop();
        delete this->ws_client;
        this->ws_client = nullptr;
    }
}