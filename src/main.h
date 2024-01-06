#include <Arduino.h>
#include <pgmspace.h>

// #define _RELEASE_BUILD

#ifndef _RELEASE_BUILD
#define _DEBUG_DEV
//#define _DEBUG_FUNCTION
#endif

#define ACTIVE_OTA
// #define OTA_PASSWORD  "password"

#define ACTIVE_SOLAR
//#define ACTIVE_WEBSOCKET

#include "memorySPIFFS.h"
#include "wifiManage.h"
#ifdef ACTIVE_OTA
#include "OTA.h"
#endif
#include "http.h"
#include "IO.h"
#include "timeManage.h"
#include "custom.h"
#ifdef ACTIVE_WEBSOCKET
#include "webSocket.h"
#endif

void restart();
void factory_reset();
uint32_t getChipID();

// DISABLE SERIAL IN RELEASE MODE
#ifdef _RELEASE_BUILD
#define _debug_print(var) yield()
#define _debug_println(var) yield()
#define _debug_printf(var) yield()
#else
#define _debug_print(var) Serial.print(var)
#define _debug_println(var) Serial.println(var)
#define _debug_printf(var) Serial.printf(var)
#endif