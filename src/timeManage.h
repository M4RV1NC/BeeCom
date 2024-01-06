#include <ESP32Time.h>
#include "time.h"

#ifdef _DEBUG_DEV
// #define _DEBUG_TIME
// #define _DEBUG_TIME_FLOW
// #define _DEBUG_RANGE
#endif

// NTP server
#define INIT_DATE 1609459200UL // 1/01/2021 00:00:00
#ifndef _DEBUG_TIME
#define EPOCH_SAVE_DELAY 600000UL // each 10 minutes
#else
#define EPOCH_SAVE_DELAY 60000L // each 1 minutes
#endif

// for custom
uint32_t time_timeToEpoch(uint8_t hours, uint8_t minutes, uint8_t seconds = 0);
uint8_t time_currentHour();
bool time_epochDelay(uint32_t timeStart, uint16_t delayMinutes, uint16_t delaySeconds = 0);
String time_showEpoch();
bool time_setTimeOffset(uint8_t hour, uint8_t minute);
int16_t time_getOffsetGMT();

// for main
void time_setup();
void time_loop();
String time_command(String data);