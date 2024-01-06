#ifdef _DEBUG_DEV
// #define _DEBUG_ONEWIRE
#endif

// https://www.upesy.fr/blogs/tutorials/esp32-pinout-reference-gpio-pins-ultimate-guide#d1bb7fb724794b96853f2f14bdecc424
//  pinouts
#define RELAY0_PIN 33
#define RELAY1_PIN 25
#define RELAY2_PIN 12//26
#define RELAY3_PIN 12//27

#define ONEWIRE_SENSORS_PIN 32

#define INPUT0_PIN 36
#define INPUT1_PIN 39
#define INPUT2_PIN 34
#define INPUT3_PIN 35
#define INPUT4_PIN_BUT_SW 14
#define INPUT5_PIN_BUT_DT 34
#define INPUT6_PIN_BUT_CLK 35

#define NB_SENSORS 4
#define NB_RELAYS 2
#define NB_INPUTS 4

struct IO_struct
{
    uint8_t pin;
    uint8_t state = 0;
    uint8_t previousState = 0;
    uint8_t change = 0;
    uint8_t falling = 0;
    uint8_t rising = 0;
};

struct rotary_button
{
    int8_t direction = 0;
    int8_t clicked = 0;   
};

struct temp_struct
{
    float temp[NB_SENSORS] = {0};
    uint8_t count = 0;
    uint32_t millisMemSensor = 0;
    const uint32_t SENSORS_DELAY = 3000; // 1s to refresh temp
    bool error[NB_SENSORS] = {false};
};

// for main
void IO_setup();
void IO_loop();

// for custom
bool IO_arg(const String argName, const String argValue);
String IO_command(String data);

// for generic
uint8_t IO_getInputState(uint8_t inputNum);
uint8_t IO_getRelayState(uint8_t relayNum);
bool IO_setRelay(uint8_t relayNum, uint8_t state);

// for solar
bool IO_poolSensorOK();
float IO_getPoolTemp();
String IO_getPoolTempS();
bool IO_airSensorOK();
float IO_getAirTemp();
String IO_getAirTempS();
bool IO_solarSensorOK();
float IO_getSolarTemp();
String IO_getSolarTempS();
void IO_setRelayPump(uint8_t state);
void IO_setRelaySolar(uint8_t state);
uint8_t IO_getRelayPumpState();
uint8_t IO_getRelaySolarState();
