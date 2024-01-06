#include "main.h"

temp_struct DS18B20;

IO_struct input[NB_INPUTS], relay[NB_RELAYS];

rotary_button button;

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONEWIRE_SENSORS_PIN);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// IO function
void IO_setRelay(IO_struct relayNum, uint8_t state)
{
    if (state == 0)
    {
        digitalWrite(relayNum.pin, 1);
        return;
    }
    digitalWrite(relayNum.pin, 0);
    return;
}

void IO_temperatureRequest(uint8_t &count) // update températuree
{
    float tempC = sensors.getTempCByIndex(count);

    // Check if reading was successful
    if (tempC != DEVICE_DISCONNECTED_C)
    {
#ifdef _DEBUG_ONEWIRE
        _debug_print("Temperature for the device ");
        _debug_print(count);
        _debug_print(" : ");
        _debug_println(tempC);
#endif
        DS18B20.error[count] = false;
        DS18B20.temp[count] = tempC;
    }

    else
    {
        DS18B20.error[count] = true;

        if (DS18B20.temp[count] > 0)
        {
            DS18B20.temp[count]--;
        }
        else
        {
            DS18B20.temp[count] = 0;
        }
#ifdef _DEBUG_ONEWIRE
        _debug_println("Error: Could not read temperature data");
#endif
    }

    count++; // next sensors

    if (count >= NB_SENSORS) // no more sensors got to first one
    {
        count = 0;
    }
    // request next sensor temp
    sensors.setWaitForConversion(false); // makes it async
    sensors.requestTemperaturesByIndex(count);

    return;
}

void IO_inputUpdate() // update state of relays
{
    for (uint8_t i = 0; i < NB_RELAYS; i++)
    {
        relay[i].rising = 0; // reset state
        relay[i].falling = 0;
        relay[i].change = 0;
        relay[i].previousState = relay[i].state;           // memorize previous state
        relay[i].state = 100 * !digitalRead(relay[i].pin); // update input state

        if (relay[i].state != relay[i].previousState) // check if change occured
        {
            relay[i].change = 100; // set change detection

            if (relay[i].state)
            {
                relay[i].rising = 100;
            }
            else
            {
                relay[i].falling = 100;
            }
        }
    }
    for (uint8_t i = 0; i < NB_INPUTS; i++)
    {
        input[i].rising = 0; // reset state
        input[i].falling = 0;
        input[i].change = 0;
        input[i].previousState = input[i].state;           // memorize previous state
        input[i].state = 100 * !digitalRead(input[i].pin); // update input state

        if (input[i].state != input[i].previousState) // check if change occured
        {
            input[i].change = 100; // set change detection

            if (input[i].state)
            {
                input[i].rising = 100;
            }
            else
            {
                input[i].falling = 100;
            }
        }
    }

    return;
}

// inputs
float IO_getPoolTemp()
{
    return DS18B20.temp[0];
}

String IO_getPoolTempS()
{
    return String(DS18B20.temp[0], 1);
}

bool IO_poolSensorOK()
{
    return DS18B20.error[0];
}

float IO_getAirTemp()
{
    return DS18B20.temp[1];
}

String IO_getAirTempS()
{
    return String(DS18B20.temp[1], 1);
}

bool IO_airSensorOK()
{
    return DS18B20.error[1];
}

float IO_getSolarTemp()
{
    return DS18B20.temp[2];
}

String IO_getSolarTempS()
{
    return String(DS18B20.temp[2], 1);
}

bool IO_solarSensorOK()
{
    return DS18B20.error[2];
}

uint8_t IO_getRelayPumpState()
{
    return relay[0].state;
}

uint8_t IO_getRelaySolarState()
{
    return relay[1].state;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
// generic
uint8_t IO_getRelayState(uint8_t relayNum)
{
    if (relayNum <= NB_RELAYS)
    {
        return relay[relayNum].state;
    }
    return 0;
}

uint8_t IO_getInputState(uint8_t inputNum)
{
    if (inputNum <= NB_INPUTS)
    {
        return input[inputNum].state;
    }
    return 0;
}

// output
void IO_setRelayPump(uint8_t state)
{
    IO_setRelay(relay[0], state);
    return;
}

void IO_setRelaySolar(uint8_t state)
{
    IO_setRelay(relay[1], state);
    return;
}

// generic
bool IO_setRelay(uint8_t relayNum, uint8_t state)
{
    if (relayNum <= NB_RELAYS)
    {
        IO_setRelay(relay[relayNum], state);
        return true;
    }
    _debug_println(F("wrong relayNum"));
    return false;
}

String IO_command(String data) // respond to custom command, for debugging
{
    String response = "no match";

    if (data.startsWith("temp"))
    {
        uint8_t tempNum = data.substring(4, 5).toInt();
        if (tempNum < NB_SENSORS)
        {
            response = "temp : " + (String)DS18B20.temp[tempNum] + "°C";
        }
    }

    if (data.startsWith("input"))
    {
        uint8_t inputNum = data.substring(5, 6).toInt();
        if (inputNum < NB_INPUTS)
        {
            response = "input : " + (String)input[inputNum].state;
        }
    }

    if (data.startsWith("relay"))
    {
        uint8_t relayNum = data.substring(5, 6).toInt();
        if (relayNum < NB_RELAYS)
        {
            if (data.substring(6, 8).equals("on"))
            {
                IO_setRelay(relay[relayNum], 1);
                response = "Turn on ";
            }
            if (data.substring(6, 9).equals("off"))
            {
                IO_setRelay(relay[relayNum], 0);
                response = "Turn off ";
            }
            response += data.substring(0, 6);
        }
    }
    return response;
}

bool IO_arg(const String argName, const String argValue) // your code here to manage arguments
{
    if (argName.startsWith("relay"))
    {
        uint8_t relayNum = argName.substring(5, 6).toInt();
        if (relayNum < NB_RELAYS)
        {
            if (argValue.equals("on"))
            {
                IO_setRelay(relay[relayNum], 1);
                return true;
            }
            if (argValue.equals("off"))
            {
                IO_setRelay(relay[relayNum], 0);
                return true;
            }
            if (argValue.equals("toggle"))
            {
                IO_setRelay(relay[relayNum], !relay[relayNum].state);
                return true;
            }
            if (argValue.equals("killall"))
            {
                for (uint8_t i = 0; i < NB_RELAYS; i++)
                {
                    IO_setRelay(relay[i], 0); // set output data to 0
                }
                return true;
            }
        }
    }
    return false;
}

void IRAM_ATTR IO_button_turning()
{
    if (button.direction > -60 && button.direction < 60)
    {
        if (!digitalRead(INPUT5_PIN_BUT_DT))
        {
            // ets_printf("+\n");
            button.direction += 1;
            return;
        }
        // ets_printf("-\n");
        button.direction -= 1;
    }
    return;
}

void IRAM_ATTR IO_button_clicked()
{
    if (button.clicked < 60)
    {

        if (digitalRead(INPUT4_PIN_BUT_SW))
        {
            // ets_printf("c\n");
            button.clicked += 1;
        }
    }
    return;
}

bool IO_buttonForward()
{
    if (button.direction > 0)
    {
        button.direction = 0;
        return true;
    }
    return false;
}

bool IO_buttonBackward()
{
    if (button.direction < 0)
    {
        button.direction = 0;
        return true;
    }
    return false;
}

bool IO_buttonClick()
{
    if (button.clicked > 0)
    {
        button.clicked = 0;
        return true;
    }
    return false;
}

void IO_setup()
{
#ifdef _DEBUG_FUNCTION
    _debug_println(__FUNCTION__);
#endif
    pinMode(ONEWIRE_SENSORS_PIN, INPUT_PULLUP);

    input[0].pin = INPUT0_PIN;
    input[1].pin = INPUT1_PIN;
    input[2].pin = INPUT2_PIN;
    input[3].pin = INPUT3_PIN;

    relay[0].pin = RELAY0_PIN;
    relay[1].pin = RELAY1_PIN;
    // relay[2].pin = RELAY2_PIN;
    // relay[3].pin = RELAY3_PIN;

    for (uint8_t i = 0; i < NB_INPUTS; i++)
    {
        pinMode(input[i].pin, INPUT_PULLUP);
    }

    for (uint8_t i = 0; i < NB_RELAYS; i++)
    {
        IO_setRelay(relay[i], 0);      // set output data to 0
        pinMode(relay[i].pin, OUTPUT); // set pin output mode
    }
    // interrupts
    pinMode(INPUT4_PIN_BUT_SW, INPUT);
    pinMode(INPUT5_PIN_BUT_DT, INPUT);
    pinMode(INPUT6_PIN_BUT_CLK, INPUT);
    attachInterrupt(INPUT6_PIN_BUT_CLK, IO_button_turning, RISING);
    attachInterrupt(INPUT4_PIN_BUT_SW, IO_button_clicked, RISING);

    // Start up the library
    sensors.begin();
    // low resolution to increase speed of reading default 9 --> +/- 0.5
    sensors.setResolution(12);
    // makes it async
    sensors.setWaitForConversion(false);
    // launch temp request process
    sensors.requestTemperaturesByIndex(0);

    return;
}

void IO_loop()
{
#ifdef _DEBUG_FUNCTION
    _debug_println(__FUNCTION__);
#endif

    // update change sensor each second
    if ((millis() - DS18B20.millisMemSensor) >= DS18B20.SENSORS_DELAY)
    {
        DS18B20.millisMemSensor = millis();
        IO_temperatureRequest(DS18B20.count);
    }

    IO_inputUpdate();



///////test
    if (IO_buttonClick())
    {
        _debug_println("click!");
    }
    if (IO_buttonForward())
    {
        _debug_println("Forward!");
    }
    if (IO_buttonBackward())
    {
        _debug_println("Backward!");
    }

    return;
}
