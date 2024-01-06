#include "main.h"

#ifdef ACTIVE_WEBSOCKET

WebSocketsServer webSocket = WebSocketsServer(81);

void webSocket_print(String data, uint8_t client)
{
#ifdef ACTIVE_WEBSOCKET
    webSocket.sendTXT(client, data); // always send data to only first client 0
#endif
    return;
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{
    switch (type)
    {
    case WStype_DISCONNECTED:
#ifndef _RELEASE_BUILD
        Serial.printf("[%u] Disconnected!\n", num);
#endif
        break;

    case WStype_CONNECTED:
    {
#ifndef _RELEASE_BUILD
        IPAddress temp = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, temp[0], temp[1], temp[2], temp[3], payload);
#endif
    }
    break;

    case WStype_TEXT:
        // Serial.printf("[%u] get Text: %s\n", num, payload);
        {
            String WScommand;
            for (uint16_t i = 0; i < length; i++)
            {
                WScommand += (char)payload[i];
            }
            // exemple manage command
            String echo = "<hr> -> " + WScommand;
            webSocket.sendTXT(num, echo);
            String response = custom_command(WScommand);
            webSocket.sendTXT(num, response);
        }
        break;
    default:
        break;
    }
    return;
}

void webSocket_setup()
{
#ifdef _DEBUG_FUNCTION
    _debug_println(__FUNCTION__);
#endif

    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
    return;
}

void webSocket_loop()
{
#ifdef _DEBUG_FUNCTION
    _debug_println(__FUNCTION__);
#endif

    webSocket.loop();
    // example
    // String toto = (String)millis();
    // webSocket_print(toto);

    return;
}
#endif