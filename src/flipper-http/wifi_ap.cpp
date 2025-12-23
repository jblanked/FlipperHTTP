#include "wifi_ap.hpp"

#ifndef BOARD_BW16
static const char *DNS_HOST = "*";
static const byte DNS_PORT = 53;
#endif

static const PROGMEM String wifiAPHeader =
    "HTTP/1.1 200 OK\r\n"
    "Content-type:text/html\r\n"
    "\r\n";

WiFiAP::WiFiAP(UART *uartClass, WiFiUtils *wifiUtils)
#ifndef BOARD_BW16
    : uart(uartClass), wifi(wifiUtils), dnsServer(), isRunning(false)
#else
    : uart(uartClass), wifi(wifiUtils), isRunning(false)
#endif
{
    html = wifiAPHeader;
    html += "<!DOCTYPE html><html>\r\n";
    html += "<head><title>FlipperHTTP</title></head>\r\n";
    html += "<body><h1>Welcome to FlipperHTTP AP Mode</h1></body>\r\n";
    html += "</html>";
}

void WiFiAP::printInputs(const String &request)
{
    auto getIdx = request.indexOf("get?");
    if (getIdx != -1 && request.startsWith("GET"))
    {
        int startPos = getIdx;
        int endPos = request.indexOf("HTTP/1.1", startPos);

        if (startPos > 0 && endPos > startPos)
        {
            String path = request.substring(startPos, endPos);
            int start = getIdx - 1;
            int end = path.indexOf('&', start);
            while (end != -1)
            {
                String part = path.substring(start, end);
                uart->println(part);
                start = end + 1;
                end = path.indexOf('&', start);
            }
            String part = path.substring(start, end);
            uart->println(part);
        }
    }
}

bool WiFiAP::start(const char *ssid)
{
    String ipStr = wifi->connectAP(ssid);
    if (ipStr.length() == 0)
    {
        uart->println(F("[ERROR] Failed to start AP mode."));
        return false;
    }

#ifndef BOARD_BW16
    apIP = IPAddress();
    apIP.fromString(ipStr);
#endif

#ifndef BOARD_BW16
    dnsServer.start(DNS_PORT, DNS_HOST, apIP);
#endif

    char buffer[64];
    snprintf(buffer, sizeof(buffer), "[SUCCESS]{\"IP\":\"%s\"}", ipStr.c_str());
    uart->println(buffer);
    uart->flush();

    isRunning = true;
    return true;
}

void WiFiAP::run()
{
    if (!isRunning)
    {
        uart->println(F("[ERROR] AP mode not running."));
        return;
    }

    WiFiServer server(80);
    server.begin();

    while (true)
    {
#ifndef BOARD_BW16
        dnsServer.processNextRequest();
#endif
        if (uart->available())
        {
            String cmd = uart->readSerialLine();
            if (cmd.startsWith("[WIFI/AP/STOP]"))
            {
                uart->println(F("[INFO] Stopping AP mode."));
                uart->flush();
                break;
            }
            else if (cmd.startsWith("[WIFI/AP/UPDATE]"))
            {
                String newHtml = uart->readStringUntilString("[WIFI/AP/UPDATE/END]");
                newHtml.trim();
                updateHTML(newHtml);
                uart->println(F("[INFO] HTML updated."));
            }
        }

#if defined(BOARD_PICO_W) || defined(BOARD_PICO_2W) || defined(BOARD_VGM) || defined(BOARD_PICOCALC_W) || defined(BOARD_PICOCALC_2W)
        WiFiClient client = server.accept();
#else
        WiFiClient client = server.available();
#endif
        if (client)
        {
            uart->println(F("[INFO] Client Connected."));
            String request = "";
            unsigned long timeout = millis() + 5000;
            while (client.connected() && millis() < timeout)
            {
                if (request.indexOf("\r\n\r\n") > 0)
                {
                    break;
                }

                // Read available data
                while (client.available() && millis() < timeout)
                {
                    char c = client.read();
                    request += c;
                    timeout = millis() + 1000; // Reset timeout on data received
                }
                delay(1);
            }
            printInputs(request);
            client.println(html);
            delay(10);
            client.stop();
        }
        delay(10);
    }

    server.end();
    wifi->disconnect();
#ifndef BOARD_BW16
    dnsServer.stop();
#endif
    isRunning = false;
}

void WiFiAP::updateHTML(const String &htmlContent)
{
    html = wifiAPHeader;
    html += htmlContent;
}
