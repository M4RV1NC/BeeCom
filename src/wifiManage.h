#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>

#ifdef _DEBUG_DEV
#define _DEBUG_WIFI
#endif

#define ESP_hostname "ESP-BASE"        // 13caract max
#define UPDATE_TIME_RECONNECT 300000UL // 5mins

bool wifi_connected();
void wifi_check();
void wifi_setup();
void wifi_loop();