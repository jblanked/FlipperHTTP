#include "wifi_ap.h"

static const PROGMEM String wifiAPHeader = "HTTP/1.1 200 OK\r\n"
                                           "Content-type:text/html\r\n"
                                           "\r\n";

WiFiAP::WiFiAP(UART *uartClass, WiFiUtils *wifiUtils) : ip(""),
                                                        uart(uartClass), wifi(wifiUtils),
                                                        isRunning(false)
{
    // default html
    this->html = wifiAPHeader;
    this->html += "<!DOCTYPE html><html>\r\n";
    this->html += "<head><title>FlipperHTTP</title></head>\r\n";
    this->html += "<body><h1>Welcome to FlipperHTTP AP Mode</h1></body>\r\n";
    this->html += "</html>";
}
bool WiFiAP::start(const char *ssid)
{
    this->ip = this->wifi->connectAP(ssid); // Connect to WiFi in AP mode
    if (this->ip != "")
    {
        char buffer[64];
        snprintf(buffer, sizeof(buffer), "[SUCCESS]{\"IP\":\"%s\"}", this->ip.c_str());
        this->uart->println(buffer);
        this->uart->flush();
        this->isRunning = true; // Set running flag to true
        return true;            // Return true if connected successfully
    }
    else
    {
        this->uart->println(F("[ERROR] Failed to start AP mode."));
        return false; // Return false if failed to connect
    }
}
void WiFiAP::run()
{
    if (!this->isRunning)
    {
        this->uart->println(F("[ERROR] AP mode is not running."));
        return; // Exit if AP mode is not running
    }
    WiFiServer server(80);
    server.begin();
    while (true)
    {
        // Check for UART command to stop AP mode.
        if (this->uart->available())
        {
            String uartCmd = this->uart->readSerialLine();
            // if starts with [WIFI/AP/STOP] then stop AP mode
            if (uartCmd.startsWith("[WIFI/AP/STOP]"))
            {
                this->uart->println(F("[INFO] Stopping AP mode."));
                this->uart->flush();
                break;
            }
            // if starts with [WIFI/AP/UPDATE] then update the HTML
            else if (uartCmd.startsWith("[WIFI/AP/UPDATE]"))
            {
                this->uart->println("DEBUG: Update command");
                String uartHTML = this->uart->readStringUntilString("[WIFI/AP/UPDATE/END]");
                uartHTML.trim();
                this->uart->println("DEBUG: Received HTML update:");
                this->uart->println(uartHTML);
                this->updateHTML(uartHTML);
            }
        }

        WiFiClient client = server.available(); // Check for incoming client
        if (client)
        {
            this->uart->println(F("[INFO] Client Connected."));
            while (client.connected())
            {
                if (client.available())
                {
                    String line = client.readStringUntil('\n');
                    if (line == "\r")
                    {
                        // End of HTTP headers; now send the response
                        client.println(this->html);
                        break;
                    }
                }
            }
            this->uart->flush(); // Flush the UART buffer
            client.stop();       // Stop the client connection
        }
        delay(10);
    }
    server.end();             // End the server
    this->wifi->disconnect(); // Disconnect from WiFi
}
void WiFiAP::updateHTML(String html)
{
    this->html = wifiAPHeader; // Reset the HTML header
    this->html += html;        // Update the HTML content
}