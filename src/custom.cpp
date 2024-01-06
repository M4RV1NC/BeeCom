#include "main.h"

schedule_struct manualSchedule[MANUAL_SCHEDULE], autoSchedule;

antiFreeze_struct airFrost, waterFrost;

uint32_t millisMemUpdate = 0, millisMemBackup = 0;

uint8_t mode = MANUAL;
bool backupNeeded = false;

bool custom_autoMode()
{
    if (mode == AUTOMATIC)
    {
        return true;
    }
    return false;
}

bool custom_manualMode()
{
    if (mode == MANUAL)
    {
        return true;
    }
    return false;
}

bool custom_remoteMode()
{
    if (mode == REMOTE)
    {
        return true;
    }
    return false;
}

String custom_formattedTime(schedule_struct object)
{
    String hoursStr = object.hours < 10 ? "0" + String(object.hours) : String(object.hours);
    String minuteStr = object.minutes < 10 ? "0" + String(object.minutes) : String(object.minutes);

    return hoursStr + ":" + minuteStr;
}

String custom_JSON(bool notSaved)
{
    StaticJsonDocument<512> doc;
    if (notSaved)
    {
        doc["device"] = ESP_hostname;
        doc["SSID"] = WiFi.SSID();
        doc["RSSI"] = WiFi.RSSI();
        doc["temp0"] = IO_getPoolTempS();
        doc["temp1"] = IO_getAirTempS();
        doc["temp2"] = IO_getSolarTempS();
        doc["input0_state"] = IO_getInputState(0);
        doc["input1_state"] = IO_getInputState(1);
        doc["input2_state"] = IO_getInputState(2);
        doc["relay0_state"] = IO_getRelayState(0);
        doc["relay1_state"] = IO_getRelayState(1);
        doc["relay2_state"] = IO_getRelayState(2);
        doc["relay3_state"] = IO_getRelayState(3);
        doc["time"] = time_showEpoch();
        doc["gmtOffset"] = time_getOffsetGMT();
    }
    doc["mode"] = mode;
    doc["autoStart"] = custom_formattedTime(autoSchedule);
    doc["autoDuration"] = autoSchedule.delay_minutes;
    doc["airFreezeSP"] = airFrost.SP;
    doc["waterFreezeSP"] = waterFrost.SP;
    doc["manStart0"] = custom_formattedTime(manualSchedule[0]);
    doc["duration0"] = manualSchedule[0].delay_minutes;
    doc["manStart1"] = custom_formattedTime(manualSchedule[1]);
    doc["duration1"] = manualSchedule[1].delay_minutes;
    doc["manStart2"] = custom_formattedTime(manualSchedule[2]);
    doc["duration2"] = manualSchedule[2].delay_minutes;

    String message;
    serializeJsonPretty(doc, message);

    return message;
}

void custom_configChange(bool state)
{
    if (state)
    {
        backupNeeded = true;
        millisMemBackup = millis();
#ifdef _DEBUG_SAVE_RESTORE
        _debug_println("backupNeeded");
#endif
        return;
    }
    backupNeeded = false;
#ifdef _DEBUG_SAVE_RESTORE
    _debug_println("backupDone");
#endif
    return;
}

bool custom_dataToSchedule(String data, schedule_struct &object) // verify and set the schedule if ok (true)
{
    int8_t hours = data.substring(0, 2).toInt();
    int8_t minutes = data.substring(3, 5).toInt();

    if (hours >= 0 && hours < 24 && minutes >= 0 && minutes < 60)
    {
        if (object.hours == hours && object.minutes == minutes) // if no change abord
        {
            return false;
        }

        object.hours = hours;
        object.minutes = minutes;
#ifdef _DEBUG_SAVE_RESTORE
        _debug_println(custom_formattedTime(object));
#endif
        custom_configChange(true);
        return true;
    }
    return false;
}

bool custom_dataToDuration(int16_t data, schedule_struct &object) // verify and set the duration if ok (true)
{
    if (data >= 0 && data < DURATION_MAX)
    {
        if (object.delay_minutes == data)
        {
            return false;
        }
        object.delay_minutes = data;
#ifdef _DEBUG_SAVE_RESTORE
        _debug_println(object.delay_minutes);
#endif
        custom_configChange(true);
        return true;
    }
    return false;
}

void custom_autoDuration() // compute automatic mode duration depending on temperature
{
    if (IO_getRelayPumpState() == 0 && !IO_poolSensorOK()) // if pump not running don't compute time to run
    {
#ifdef _DEBUG_SCHEDULE
        _debug_println("pump off no duration compute or pool sensor is dead");
#endif
        return;
    }
    else if ((autoSchedule.hours + 1) < time_currentHour()) // pump is running but already 1 hour
    {
#ifdef _DEBUG_SCHEDULE
        _debug_println("time too late to compute duration");
#endif
        return;
    }

    autoSchedule.delay_minutes = nearbyint(60.0 * pow(e, 0.1 * IO_getPoolTemp()));

    if (autoSchedule.delay_minutes < 60)
    {
        autoSchedule.delay_minutes = 60;
    }

    if (autoSchedule.delay_minutes > 900)
    {
        autoSchedule.delay_minutes = 900;
    }

#ifdef _DEBUG_SCHEDULE
    _debug_print("autoSchedule.delay_minutes : ");
    _debug_println(autoSchedule.delay_minutes);
#endif
    return;
}

void custom_modeChange(uint8_t new_mode) // set mode
{
    if (mode == new_mode) // if no change, abord
    {
        return;
    }

    mode = new_mode;

    custom_configChange(true);

#ifdef _DEBUG_MODE
    switch (new_mode)
    {
    case MANUAL:
        _debug_print("manual");
        break;
    case AUTOMATIC:
        _debug_print("automatic");
        break;
    case REMOTE:
        _debug_print("remote");
        break;
    default:
        _debug_print("new_mode error");
        return;
        break;
    }
    _debug_printn(" mode");
#endif

    return;
}

bool custom_antiFreezeSet(float newValue, antiFreeze_struct &object) // verify and set the antiFreeze_struct value if ok (true)
{
    if (newValue <= ANTIFREEZE_MAX && newValue >= ANTIFREEZE_MIN)
    {
        if (newValue == object.SP) // if no change, abord
        {
            return false;
        }
        object.SP = newValue;
#ifdef _DEBUG_FREEZE
        _debug_print("antiFreeze : ");
        _debug_println(newValue);
#endif
        custom_configChange(true);
        return true;
    }
    return false;
}

void custom_arg(const String argName, const String argValue) // your code here to manage arguments
{
#ifdef _DEBUG_ARG
    _debug_println(argName + " : " + argValue);
#endif
    // actions
    if (custom_remoteMode())
    {
        if (IO_arg(argName, argValue))
        {
            return;
        }
    }
    // from config
    if (argName.equals("autoMode") && argValue.equals("true"))
    {
        custom_modeChange(AUTOMATIC);
        return;
    }
    if (argName.equals("manMode") && argValue.equals("true"))
    {
        custom_modeChange(MANUAL);
        return;
    }
    if (argName.equals("remoteMode") && argValue.equals("true"))
    {
        custom_modeChange(REMOTE);
        return;
    }
    if (argName.equals("airFreezeSP"))
    {
        custom_antiFreezeSet(argValue.toFloat(), airFrost);
        return;
    }
    if (argName.equals("waterFreezeSP"))
    {
        custom_antiFreezeSet(argValue.toFloat(), waterFrost);
        return;
    }
    if (argName.equals("autoStart"))
    {
        custom_dataToSchedule(argValue, autoSchedule);
        return;
    }
    if (argName.startsWith("manStart"))
    {
        uint16_t i = argName.substring(8, 9).toInt();
        if (i >= 0 && i < MANUAL_SCHEDULE) // wrong parameter, abord
        {
            custom_dataToSchedule(argValue, manualSchedule[i]);
        }
        return;
    }
    if (argName.startsWith("duration"))
    {
        uint16_t i = argName.substring(8, 9).toInt();
        if (i >= 0 && i < MANUAL_SCHEDULE) // wrong parameter, abord
        {
            custom_dataToDuration(argValue.toInt(), manualSchedule[i]);
        }
        return;
    }
    if (argName.equals("currentTime"))
    {
        time_setTimeOffset(argValue.substring(0, 2).toInt(), argValue.substring(3, 5).toInt());
        custom_configChange(true);
        return;
    }
    return;
}

bool custom_scheduleReq(schedule_struct &epoch) // return if range is matched
{
    if (epoch.delay_minutes <= 0)
    {
        epoch.req = false;
        return false;
    }
    epoch.req = time_epochDelay(time_timeToEpoch(epoch.hours, epoch.minutes), epoch.delay_minutes);
    return true;
}

void custom_restore()
{
    String dummy;

    if (FS_jsonRead(dummy)) // file is readable
    {
        StaticJsonDocument<768> doc;
        DeserializationError error = deserializeJson(doc, dummy);

        if (error)
        {
#ifdef _DEBUG_SAVE_RESTORE
            _debug_print(F("deserializeJson() failed: "));
            _debug_println(error.c_str());
#endif
            return;
        }

        // update from ROM
        mode = doc["mode"];

        custom_dataToSchedule(doc["autoStart"], autoSchedule);
        custom_antiFreezeSet(doc["airFreezeSP"], airFrost);
        custom_antiFreezeSet(doc["waterFreezeSP"], waterFrost);

        custom_dataToSchedule(doc["manStart0"], manualSchedule[0]);
        custom_dataToDuration(doc["duration0"], manualSchedule[0]);

        custom_dataToSchedule(doc["manStart1"], manualSchedule[1]);
        custom_dataToDuration(doc["duration1"], manualSchedule[1]);

        custom_dataToSchedule(doc["manStart2"], manualSchedule[2]);
        custom_dataToDuration(doc["duration2"], manualSchedule[2]);

        custom_configChange(false);
        return;
    }
    _debug_print(F("FS_jsonRead() failed"));
    return;
}

void custom_backup()
{
    FS_jsonWrite(custom_JSON(false));

    custom_configChange(false);

    return;
}

String custom_command(String data) // respond to custom command, for debugging
{
    String response = "no match";

    if (data.equals("backup"))
    {
        custom_backup();
        response = "ok";
    }

    if (data.equals("restore"))
    {
        custom_restore();
        response = "ok";
    }

    if (data.equals("show"))
    {
        response = custom_JSON(true);
    }

    if (data.startsWith("factoryReset"))
    {
        factory_reset();
        response = "factory reset...";
    }

    if (response.startsWith("no match"))
    {
        response = IO_command(data);
    }

    if (response.startsWith("no match"))
    {
        response = FS_command(data);
    }

    if (response.startsWith("no match"))
    {
        response = time_command(data);
    }

    return response;
}

bool custom_noFrostReq(antiFreeze_struct &object, bool timerManaged = false)
{
    if (object.SP != 10.0) // antiFreezeSP is active
    {
        if (object.req)
        {
            if (timerManaged)
            {
                if (millis() >= (object.time + 36000L * object.T_OFF))
                {
#ifdef _DEBUG_FREEZE
                    _debug_println("antiFreeze T OFF");
#endif
                    object.req = false;
                    object.time = millis();
                    return true;
                }
#ifdef _DEBUG_FREEZE2
                _debug_println("antiFreeze T OFF condition missing");
#endif
            }
            if (object.PV >= (object.SP + 0.5))
            {
                object.req = false;
                object.time = millis();
#ifdef _DEBUG_FREEZE
                _debug_println("antiFreeze off");
#endif
                return true;
            }
        }
        else
        {
            if (object.PV <= object.SP)
            {
                if (timerManaged)
                {
                    if (millis() >= (object.time + 36000L * object.T_ON))
                    {
#ifdef _DEBUG_FREEZE
                        _debug_println("antiFreeze T ON");
#endif
                        object.req = true;
                        object.time = millis();
                        return true;
                    }
#ifdef _DEBUG_FREEZE2
                    _debug_println("antiFreeze T ON condition missing");
#endif
                    return true;
                }
                object.req = true;
                object.time = millis();
#ifdef _DEBUG_FREEZE
                _debug_println("antiFreeze on");
#endif
                return true;
            }
        }
    }
    else
    {
        if (object.req)
        {
#ifdef _DEBUG_FREEZE

            _debug_println("antiFreeze disable");
#endif
            object.req = false;
            object.time = 0L;
        }
    }
    return false;
}

void custom_setup() // your setup here
{
    airFrost.T_OFF = 50; // 0.5 hour to stop if air is too cold
    airFrost.T_ON = 100; // 1 hour to restart cycle if air is still too cold
#ifdef _DEBUG_FREEZE
    airFrost.T_OFF = 1;
    airFrost.T_ON = 2;
#endif
    airFrost.time = millis(); // init timestamp of airFrost

    // restore data
    custom_restore();

    autoSchedule.delay_minutes = 888;

    return;
}

void custom_loop() // your loop code here
{
#ifdef _DEBUG_FUNCTION
    _debug_println(__FUNCTION__);
#endif

    // update each UPDATE_TIME
    if ((millis() - millisMemUpdate) >= UPDATE_TIME)
    {
        millisMemUpdate = millis();

        custom_autoDuration(); // compute time to run the pump

        // assign temp to correct PV
        waterFrost.PV = IO_getPoolTemp();
        airFrost.PV = IO_getAirTemp();

        // update Req
        if (IO_airSensorOK())
        {
            custom_noFrostReq(airFrost, true);
        }

        if (IO_poolSensorOK())
        {
            custom_noFrostReq(waterFrost);
        }

        custom_scheduleReq(autoSchedule);

        for (uint8_t i = 0; i < MANUAL_SCHEDULE; i++)
        {
            custom_scheduleReq(manualSchedule[i]);
        }
#ifdef _DEBUG_SCHEDULE
        _debug_println("UPDATE_TIME");
        if (airFrost.req)
        {
            _debug_println("airFrost.req");
        }
        if (waterFrost.req)
        {
            _debug_println("waterFrost.req");
        }
        if (autoSchedule.req)
        {
            _debug_println("autoSchedule.req");
        }
        if (manualSchedule[0].req || manualSchedule[1].req || manualSchedule[2].req)
        {
            _debug_println("manualSchedule[X].req");
            _debug_println();
        }
#endif
        bool frostReq = airFrost.req || waterFrost.req;

        // following mode to update schedule req
        switch (mode)
        {
        case MANUAL:                                                                                              // manual
            IO_setRelayPump(manualSchedule[0].req || manualSchedule[1].req || manualSchedule[2].req || frostReq); // update relay state
            break;
        case AUTOMATIC: // automatic
            if (autoSchedule.delay_minutes == 0)
            {
                // init value
                autoSchedule.delay_minutes = 60;
            }
            IO_setRelayPump(autoSchedule.req || frostReq); // update relay state
            break;
        case REMOTE:                                             // remote
            IO_setRelayPump(IO_getRelayPumpState() || frostReq); // update relay state
            break;
        default:
            break;
        }
    }

    // update each hour
    if ((millis() - millisMemBackup) >= BACKUP_DELAY)
    {
        millisMemBackup = millis();

        if (backupNeeded) // update after modification, one hour later
        {
            custom_backup();
        }
    }
    return;
}

void custom_factoryReset()
{
    custom_backup();
    return;
}
