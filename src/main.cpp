#include "main.h"


void restart()
{
  _debug_println(F("Restarting in 2 seconds"));
  for (uint8_t i = 0; i <= 10; i++)
  {
    delay(200);
  }
  ESP.restart();
  return;
}

void factory_reset()
{
  FS_erase();
  custom_factoryReset();
  restart();
}

uint32_t getChipID()
{
  uint32_t chipId = 0;
  for (int i = 0; i < 17; i = i + 8)
  {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }

  return chipId;
}

void command()
{
  if (Serial.available())
  {
    String data = Serial.readString();

    if (data.equals("factoryreset"))
    {
      factory_reset();
      return;
    }

    if (data.equals("restart"))
    {
      restart();
      return;
    }

#ifdef _DEBUG_DEV
    _debug_println(custom_command(data));
#endif
  }
  return;
}

void setup()
{
  Serial.begin(115200);
  Serial.setTimeout(200); // 200ms to read UART buffer

#ifdef _DEBUG_DEV
  Serial.printf("\n\nESP32 Chip model = %s Rev %d\n", ESP.getChipModel(), ESP.getChipRevision());
  Serial.printf("This chip has %d cores\n", ESP.getChipCores());
  Serial.printf("Chip ID: %d\n\n", getChipID());
#endif

  delay(500);

  FS_setup();

  wifi_setup();

#ifdef ACTIVE_OTA
  OTA_setup();
#endif

  http_setup();

  IO_setup();

  time_setup();

  custom_setup();


#ifdef ACTIVE_WEBSOCKET
  webSocket_setup();
#endif

#ifdef ACTIVE_BLUETOOTH
  BT_setup();
#endif
  return;
}

void loop()
{
#ifdef _DEBUG_FUNCTION
  _debug_println(__FUNCTION__);
#endif

  command();

  wifi_loop();

#ifdef ACTIVE_OTA
  OTA_loop();
#endif

  http_loop();

  IO_loop();

  time_loop();

  custom_loop();

#ifdef ACTIVE_WEBSOCKET
  webSocket_loop();
#endif

  return;
}