#include "main.h"

uint32_t timeWithoutConnectionMem = 0;

bool wifiConfigured = false;

String _SSID = ESP_hostname, _PASSWORD = "";

bool wifi_connected()
{
    //_debug_println(F("wifi_connected()"));
    return (WiFi.status() == WL_CONNECTED) ? true : false;
}

bool wifi_STA_mode()
{
    WiFi.disconnect(true);
    WiFi.hostname(ESP_hostname);
    WiFi.mode(WIFI_STA);
    WiFi.begin(_SSID.c_str(), _PASSWORD.c_str()); // Connect to the WiFi network
    _debug_println("Connecting to " + _SSID);     // + "@" + _PASSWORD);

    for (uint8_t i = 0; i < 40; i++) // 20sec to connect
    {
        if (wifi_connected())
        {
            return true;
        }
        delay(500);
    }
    _debug_println(F("Connection Failed\n"));
    return false;
}

void wifi_AP_mode()
{
    WiFi.disconnect();
    WiFi.hostname(ESP_hostname);
    WiFi.mode(WIFI_AP);
    WiFi.softAP("ESP" + String(getChipID()));
    return;
}

void wifi_connection()
{
    if (wifiConfigured) // if wifi parameter exist
    {
        WiFi.setAutoReconnect(true);
        WiFi.persistent(true);

        if (wifi_STA_mode()) // try to connect
        {
            delay(500);
            return;
        }
    }

    wifi_AP_mode(); // else station mode
    delay(500);
    return;
}

void wifi_check()
{
    if (millis() - timeWithoutConnectionMem > UPDATE_TIME_RECONNECT)
    {
        timeWithoutConnectionMem = millis(); // update current millis to do not restart immediatly after connection lost
        //_debug_println(F("wifi_check()"));
        if (!wifi_connected() && wifiConfigured) // if wifi configured but not connected,
        {
            _debug_println(F("Try to reconnect wifi"));
            wifi_connection();
        }
        return;
    }
    return;
}

void WiFiEvent(WiFiEvent_t event)
{
    _debug_print("-");

    switch (event)
    {
    case ARDUINO_EVENT_WIFI_READY:
        _debug_println(F("WiFi interface ready"));
        break;
    case ARDUINO_EVENT_WIFI_SCAN_DONE:
        _debug_println(F("Completed scan for access points"));
        break;
    case ARDUINO_EVENT_WIFI_STA_START:
        _debug_println(F("WiFi client started"));
        break;
    case ARDUINO_EVENT_WIFI_STA_STOP:
        _debug_println(F("WiFi clients stopped"));
        break;
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
        _debug_print(F("Connected, RSSI: "));
        _debug_print(WiFi.RSSI());
        _debug_print(F("dBi MAC: "));
        _debug_println(WiFi.macAddress());

        break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
        _debug_println(F("Disconnected from WiFi access point"));
        break;
    case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE:
        _debug_println(F("Authentication mode of access point has changed"));
        break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
        _debug_print(F("IP: "));
        _debug_println(WiFi.localIP());
        break;
    case ARDUINO_EVENT_WIFI_STA_LOST_IP:
        _debug_println(F("Lost IP address and IP address is reset to 0"));
        break;
        /*  case ARDUINO_EVENT_WPS_ER_SUCCESS:
              _debug_println(F("WiFi Protected Setup (WPS): succeeded in enrollee mode"));
              break;
          case ARDUINO_EVENT_WPS_ER_FAILED:
              _debug_println(F("WiFi Protected Setup (WPS): failed in enrollee mode"));
              break;
          case ARDUINO_EVENT_WPS_ER_TIMEOUT:
              _debug_println(F("WiFi Protected Setup (WPS): timeout in enrollee mode"));
              break;
          case ARDUINO_EVENT_WPS_ER_PIN:
              _debug_println(F("WiFi Protected Setup (WPS): pin code in enrollee mode"));
              break;*/
    case ARDUINO_EVENT_WIFI_AP_START:
        _debug_print(F("WiFi AP started IP: "));
        _debug_println(WiFi.softAPIP());

        break;
    case ARDUINO_EVENT_WIFI_AP_STOP:
        _debug_println(F("WiFi access point stopped"));
        break;
    case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
        _debug_println(F("Client connected"));
        break;
    case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
        _debug_println(F("Client disconnected"));
        break;
    case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:
        _debug_println(F("Assigned IP address to client"));
        break;
    case ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED:
        _debug_println(F("Received probe request"));
        break;
    case ARDUINO_EVENT_WIFI_AP_GOT_IP6:
        _debug_println(F("AP IPv6 is preferred"));
        break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP6:
        _debug_println(F("STA IPv6 is preferred"));
        break;
    default:
        _debug_print(F("Wifi unknow event : "));
        _debug_println(event);
        break;
    }
    return;
}

void wifi_setup()
{
#ifdef _DEBUG_WIFI
    WiFi.onEvent(WiFiEvent);
#endif

    wifiConfigured = FS_wifiRead(_SSID, _PASSWORD);

    wifi_connection();
}

void wifi_loop()
{
    wifi_check();
}