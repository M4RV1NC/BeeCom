#include <ArduinoJson.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#ifdef _DEBUG_DEV
//#define _DEBUG_ARG
//#define _DEBUG_ONEWIRE
//#define _DEBUG_SCHEDULE
//#define _DEBUG_SAVE_RESTORE
//#define _DEBUG_MODE
//#define _DEBUG_REQ
//#define _DEBUG_FREEZE
//#define _DEBUG_TIME
#endif

#define ANTIFREEZE_MIN -10.0
#ifdef _DEBUG_FREEZE
#define ANTIFREEZE_MAX 40.0
#else
#define ANTIFREEZE_MAX 10.0
#endif
#define DURATION_MAX 1440
#define MANUAL_SCHEDULE 3
#define UPDATE_TIME 10000UL // each 10s
#define BACKUP_DELAY 600000UL // delay 10 minutes to save parameters in ROM

const float e = 2.71828182846;

enum MODE
{
    MANUAL,
    AUTOMATIC,
    REMOTE,
    MAX_ENUM_MODE
};

struct schedule_struct
{
    uint8_t hours = 0;
    uint8_t minutes = 0;
    uint16_t delay_minutes = 0;
    bool req = false;
};

struct antiFreeze_struct
{
    float SP = 10.0; // set point
    float PV = 0.0;  // process value
    bool req = 0;
    uint32_t time = 0L;
    uint8_t T_ON = 0; // 0.01 hour, max 2.55H
    uint8_t T_OFF = 0;
};

// for main
void custom_setup();
void custom_loop();
String custom_command(String data);
void custom_factoryReset();

// for http
void custom_root_acces();
void custom_arg(const String argName, const String argValue);
String custom_JSON(bool noSaved = true);

// for others
bool custom_autoMode();
bool custom_manualMode();
bool custom_remoteMode();
