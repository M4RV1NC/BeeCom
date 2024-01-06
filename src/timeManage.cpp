#include "main.h"

uint32_t epochSaveTimeMem = 0;

// Define Virtul RTC timer
ESP32Time rtcTime;

#ifdef _DEBUG_TIME_FLOW
uint32_t debugFlowTimeMem = 0;
#endif

uint32_t time_timeToEpoch(uint8_t hours, uint8_t minutes, uint8_t seconds)
{
    uint32_t epochTime;
    epochTime = hours * 3600;
    epochTime += minutes * 60;
    epochTime += seconds;
    return epochTime;
}

uint32_t time_getShortEpoch()
{
    return time_timeToEpoch(rtcTime.getHour(), rtcTime.getMinute(), rtcTime.getSecond());
}

bool time_epochInclude(uint32_t timeStart, uint32_t timeStop, uint32_t time)
{
    if (time >= timeStart && time < timeStop)
    {
        return true;
    }

    return false;
}

bool time_epochDelay(uint32_t timeStart, uint16_t delayMinutes, uint16_t delaySeconds)
{
    uint32_t time = time_getShortEpoch();
    uint32_t timeStop = timeStart + 60 * delayMinutes + delaySeconds;
    uint32_t tomorrowTimeStop = 0;

    if (timeStop >= 86400UL) // stop after midnight
    {
        tomorrowTimeStop = timeStop - 86400UL;
    }

#ifdef _DEBUG_RANGE
    _debug_print("time : ");
    _debug_println(time);
    _debug_print("timeStart : ");
    _debug_println(timeStart);
    _debug_print("timeStop : ");
    _debug_println(timeStop);
    _debug_print("tomorrowTimeStop : ");
    _debug_println(tomorrowTimeStop);
    _debug_println();
#endif
    if ((time >= timeStart && time < timeStop) || time < tomorrowTimeStop)
    {
        return true;
    }

    return false;
}

String time_showEpoch()
{
    return rtcTime.getTime();
}

uint8_t time_currentHour()
{
    return rtcTime.getHour();
}

void time_setGMTOffset(int32_t minute)
{
    if (minute > 1440 || minute < -1440)
    {
        _debug_println("offset wrong value"); // bad parameter
        return;
    }
    rtcTime.offset = minute * 60;
}

bool time_setTimeOffset(uint8_t hour, uint8_t minute)
{
#ifdef _DEBUG_TIME
    _debug_println("hour : " + String(hour) + " minute : " + String(minute));
#endif

    // rtc
    uint32_t offset = 60 * (hour - rtcTime.getHour(true)) + minute - rtcTime.getMinute();
    offset += rtcTime.offset / 60;
#ifdef _DEBUG_TIME
    _debug_println("rtcTime.offset: " + String(rtcTime.offset / 60));
    _debug_println("offset: " + String(offset));
#endif
    time_setGMTOffset(offset);

    return true;
}

void time_setTime(int32_t epoch) // default if no value
{
    rtcTime.setTime(epoch);
}

int16_t time_getOffsetGMT() // minutes
{
    return rtcTime.offset / 60;
}

void time_save()
{
    if (FS_epochWrite(rtcTime.getLocalEpoch(), time_getOffsetGMT()))
    {
#ifdef _DEBUG_TIME
        _debug_println(F("RTC saved"));
#endif
    }
    return;
}

void time_restore()
{
    uint32_t epoch = INIT_DATE;
    int16_t gmt = 0;

#ifdef _DEBUG_TIME_FLOW
    _debug_print("RTC1: ");
    _debug_println(rtcTime.getTime());
    _debug_println();
#endif

    if (FS_epochRead(epoch, gmt))
    {
#ifdef _DEBUG_TIME
        _debug_println(F("RTC restored\n"));
#endif
    }
    else
    {
#ifdef _DEBUG_TIME
        _debug_print(F("RTC restore failed"));
#endif
        time_save();
    }
    time_setTime(epoch);
    time_setGMTOffset(gmt);
    return;
}

String time_command(String data) // respond to custom command, for debugging
{
    String response = "no match.";
    if (data.startsWith("showEpoch"))
    {
        response = time_showEpoch();
    }
    if (data.startsWith("showLocalEpoch"))
    {
        response = String(rtcTime.getLocalEpoch());
    }

    if (data.startsWith("showoffset"))
    {
        response = String(time_getOffsetGMT());
    }
    if (data.startsWith("time_save"))
    {
        time_save();
        response = "done";
    }
    if (data.startsWith("time_restore"))
    {
        time_restore();
        response = "done";
    }

    return response;
}

void time_setup()
{
#ifdef _DEBUG_FUNCTION
    _debug_println(__FUNCTION__);
#endif
    // RTC
    configTime(0, 0, "pool.ntp.org");

    time_restore();

    return;
}

void time_loop()
{
#ifdef _DEBUG_FUNCTION
    _debug_println(__FUNCTION__);
#endif

    if (millis() - epochSaveTimeMem > EPOCH_SAVE_DELAY)
    {
        time_save();
        epochSaveTimeMem = millis();
    }

#ifdef _DEBUG_TIME_FLOW
    if (millis() - debugFlowTimeMem > 2000)
    {
        _debug_print("RTC: ");
        _debug_println(rtcTime.getTime());
        _debug_println();
        debugFlowTimeMem = millis();
    }
#endif

    return;
}