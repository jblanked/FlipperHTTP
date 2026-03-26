#include "command.hpp"

String commandToString(CommandType command)
{
    switch (command)
    {
    case COMMAND_TYPE_LIST:
        return "[LIST]";
    case COMMAND_TYPE_PING:
        return "[PING]";
    case COMMAND_TYPE_REBOOT:
        return "[REBOOT]";
    case COMMAND_TYPE_WIFI_IP:
        return "[WIFI/IP]";
    case COMMAND_TYPE_WIFI_SCAN:
        return "[WIFI/SCAN]";
    case COMMAND_TYPE_WIFI_SAVE:
        return "[WIFI/SAVE]";
    case COMMAND_TYPE_WIFI_CONNECT:
        return "[WIFI/CONNECT]";
    case COMMAND_TYPE_WIFI_DISCONNECT:
        return "[WIFI/DISCONNECT]";
    case COMMAND_TYPE_WIFI_LIST:
        return "[WIFI/LIST]";
    case COMMAND_TYPE_GET:
        return "[GET]";
    case COMMAND_TYPE_GET_HTTP:
        return "[GET/HTTP]";
    case COMMAND_TYPE_POST_HTTP:
        return "[POST/HTTP]";
    case COMMAND_TYPE_PUT_HTTP:
        return "[PUT/HTTP]";
    case COMMAND_TYPE_DELETE_HTTP:
        return "[DELETE/HTTP]";
    case COMMAND_TYPE_GET_BYTES:
        return "[GET/BYTES]";
    case COMMAND_TYPE_POST_BYTES:
        return "[POST/BYTES]";
    case COMMAND_TYPE_POST_FILE:
        return "[POST/FILE]";
    case COMMAND_TYPE_PARSE:
        return "[PARSE]";
    case COMMAND_TYPE_PARSE_ARRAY:
        return "[PARSE/ARRAY]";
    case COMMAND_TYPE_LED_ON:
        return "[LED/ON]";
    case COMMAND_TYPE_LED_OFF:
        return "[LED/OFF]";
    case COMMAND_TYPE_IP_ADDRESS:
        return "[IP/ADDRESS]";
    case COMMAND_TYPE_WIFI_AP:
        return "[WIFI/AP]";
    case COMMAND_TYPE_VERSION:
        return "[VERSION]";
    case COMMAND_TYPE_DEAUTH:
        return "[DEAUTH]";
    case COMMAND_TYPE_DEAUTH_STOP:
        return "[DEAUTH/STOP]";
    case COMMAND_TYPE_WIFI_STATUS:
        return "[WIFI/STATUS]";
    case COMMAND_TYPE_WIFI_SSID:
        return "[WIFI/SSID]";
    case COMMAND_TYPE_BOARD_NAME:
        return "[BOARD/NAME]";
    case COMMAND_TYPE_SOCKET_START:
        return "[SOCKET/START]";
    case COMMAND_TYPE_SOCKET_STOP:
        return "[SOCKET/STOP]";
    default:
        return "[UNKNOWN]";
    };
}

CommandType commandFromString(const String &string)
{
    if (string.length() == 0)
    {
        return COMMAND_TYPE_UNKNOWN;
    }
    if (string.startsWith("[LIST]"))
    {
        return COMMAND_TYPE_LIST;
    }
    if (string.startsWith("[PING]"))
    {
        return COMMAND_TYPE_PING;
    }
    if (string.startsWith("[REBOOT]"))
    {
        return COMMAND_TYPE_REBOOT;
    }
    if (string.startsWith("[WIFI/IP]"))
    {
        return COMMAND_TYPE_WIFI_IP;
    }
    if (string.startsWith("[WIFI/SCAN]"))
    {
        return COMMAND_TYPE_WIFI_SCAN;
    }
    if (string.startsWith("[WIFI/SAVE]"))
    {
        return COMMAND_TYPE_WIFI_SAVE;
    }
    if (string.startsWith("[WIFI/CONNECT]"))
    {
        return COMMAND_TYPE_WIFI_CONNECT;
    }
    if (string.startsWith("[WIFI/DISCONNECT]"))
    {
        return COMMAND_TYPE_WIFI_DISCONNECT;
    }
    if (string.startsWith("[WIFI/LIST]"))
    {
        return COMMAND_TYPE_WIFI_LIST;
    }
    if (string.startsWith("[GET]"))
    {
        return COMMAND_TYPE_GET;
    }
    if (string.startsWith("[GET/HTTP]"))
    {
        return COMMAND_TYPE_GET_HTTP;
    }
    if (string.startsWith("[POST/HTTP]"))
    {
        return COMMAND_TYPE_POST_HTTP;
    }
    if (string.startsWith("[PUT/HTTP]"))
    {
        return COMMAND_TYPE_PUT_HTTP;
    }
    if (string.startsWith("[DELETE/HTTP]"))
    {
        return COMMAND_TYPE_DELETE_HTTP;
    }
    if (string.startsWith("[GET/BYTES]"))
    {
        return COMMAND_TYPE_GET_BYTES;
    }
    if (string.startsWith("[POST/BYTES]"))
    {
        return COMMAND_TYPE_POST_BYTES;
    }
    if (string.startsWith("[POST/FILE]"))
    {
        return COMMAND_TYPE_POST_FILE;
    }
    if (string.startsWith("[PARSE]"))
    {
        return COMMAND_TYPE_PARSE;
    }
    if (string.startsWith("[PARSE/ARRAY]"))
    {
        return COMMAND_TYPE_PARSE_ARRAY;
    }
    if (string.startsWith("[LED/ON]"))
    {
        return COMMAND_TYPE_LED_ON;
    }
    if (string.startsWith("[LED/OFF]"))
    {
        return COMMAND_TYPE_LED_OFF;
    }
    if (string.startsWith("[IP/ADDRESS]"))
    {
        return COMMAND_TYPE_IP_ADDRESS;
    }
    if (string.startsWith("[WIFI/AP]"))
    {
        return COMMAND_TYPE_WIFI_AP;
    }
    if (string.startsWith("[VERSION]"))
    {
        return COMMAND_TYPE_VERSION;
    }
    if (string.startsWith("[DEAUTH]"))
    {
        return COMMAND_TYPE_DEAUTH;
    }
    if (string.startsWith("[DEAUTH/STOP]"))
    {
        return COMMAND_TYPE_DEAUTH_STOP;
    }
    if (string.startsWith("[WIFI/STATUS]"))
    {
        return COMMAND_TYPE_WIFI_STATUS;
    }
    if (string.startsWith("[WIFI/SSID]"))
    {
        return COMMAND_TYPE_WIFI_SSID;
    }
    if (string.startsWith("[BOARD/NAME]"))
    {
        return COMMAND_TYPE_BOARD_NAME;
    }
    if (string.startsWith("[SOCKET/START]"))
    {
        return COMMAND_TYPE_SOCKET_START;
    }
    if (string.startsWith("[SOCKET/STOP]"))
    {
        return COMMAND_TYPE_SOCKET_STOP;
    }

    return COMMAND_TYPE_UNKNOWN;
}