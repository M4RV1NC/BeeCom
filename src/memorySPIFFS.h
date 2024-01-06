#include <ArduinoJson.h>

#define USE_LittleFS

#include <FS.h>
#ifdef USE_LittleFS
#define SPIFFS LITTLEFS
#include <LITTLEFS.h>
#else
#include <SPIFFS.h>
#endif

#define FILE_MAX_SIZE 768
#define WIFI_FILE_MAX_SIZE 128

#ifdef _DEBUG_DEV
//#define _DEBUG_SPIFFS
//#define _DEBUG_SPIFFS_JSON
#endif

// for others
void FS_setup();
void FS_erase();
String FS_command(String data);

// for custom
bool FS_wifiWrite(String SSID, String PWD);
bool FS_wifiRead(String &SSID, String &PWD);
bool FS_jsonWrite(String SerializedJSON);
bool FS_jsonRead(String &SerializedJSON);
bool FS_epochWrite(uint32_t epoch, int16_t gmtOffset);
bool FS_epochRead(uint32_t &epoch, int16_t &gmtOffset);
void FS_deleteConfig();
void FS_deleteWifi();
String FS_read(String fileName);
