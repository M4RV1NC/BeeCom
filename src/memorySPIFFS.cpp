#include "main.h"

void listDir(fs::FS &fs, const char *dirname, uint8_t levels)
{
    Serial.printf("Listing directory: %s\r\n", dirname);

    File root = fs.open(dirname);
    if (!root)
    {
        Serial.println("- failed to open directory");
        return;
    }
    if (!root.isDirectory())
    {
        Serial.println(" - not a directory");
        return;
    }

    File file = root.openNextFile();
    while (file)
    {
        if (file.isDirectory())
        {
            Serial.print("  dir : ");
            Serial.println(file.name());
            if (levels)
            {
                listDir(fs, file.path(), levels - 1);
            }
        }
        else
        {
            Serial.print("file : ");
            Serial.print(file.name());
            Serial.print("\tsize : ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
    Serial.println();
    return;
}

void FS_setup()
{
#ifdef _DEBUG_FUNCTION
    _debug_println(__FUNCTION__);
#endif
    if (LittleFS.begin(true))
    {
#ifdef _DEBUG_SPIFFS
        _debug_println(F("\nFS system mounted with success"));
    }
    else
    {
        _debug_println(F("\nAn Error has occurred while mounting FS"));
#endif
    }

#ifndef _DEBUG_SPIFFS
    return;
#endif

    listDir(LittleFS, "/", 3);
    return;
}

void FS_erase()
{
    LittleFS.format();
}

void FS_deleteConfig()
{
    LittleFS.remove("/json.txt");
}

void FS_deleteWifi()
{
    LittleFS.remove("/wifiParam.txt");
}

void FS_deleteEpoch()
{
    LittleFS.remove("/epoch.txt");
}

bool FS_wifiWrite(String SSID, String PWD)
{
    File wifiFile = LittleFS.open("/wifiParam.txt", "w");

    if (!wifiFile)
    {
        _debug_println(F("Failed to open wifiParam.txt for reading"));
        wifiFile.close();
        return false;
    }

    size_t wifiFileSize = wifiFile.size();
    if (wifiFileSize > WIFI_FILE_MAX_SIZE)
    {
        _debug_println(F("wifiParam.txt size is too large"));
        wifiFile.close();
        return false;
    }

    StaticJsonDocument<WIFI_FILE_MAX_SIZE> doc;
    doc["SSID"] = SSID;
    doc["PWD"] = PWD;

    String SerializedJSON;
    serializeJson(doc, SerializedJSON);

    wifiFile.println(SerializedJSON); // writting serialized JSON into /wifiParam.txt
    wifiFile.close();

    return true;
}

bool FS_wifiRead(String &SSID, String &PWD)
{
    File wifiFile = LittleFS.open("/wifiParam.txt", "r");
    if (!wifiFile)
    {
        _debug_println(F("Failed to open wifiParam.txt for reading"));
        wifiFile.close();
        return false;
    }

    String buffer = wifiFile.readString();
    wifiFile.close();

    StaticJsonDocument<WIFI_FILE_MAX_SIZE> doc;
    DeserializationError error = deserializeJson(doc, buffer);

    if (error)
    {
        _debug_print(F("deserializeJson() failed: "));
        _debug_println(error.c_str());
        return false;
    }

    SSID = strdup(doc["SSID"]);
    PWD = strdup(doc["PWD"]);
    return true;
}

bool FS_jsonWrite(String SerializedJSON) //(String fileToOpen, String SerializedJSON)
{
    File jsonFile = LittleFS.open("/json.txt", "w");
    if (!jsonFile)
    {
        _debug_println(F("Failed to open json.txt for writing"));
        jsonFile.close();
        return false;
    }
    size_t jsonFileSize = jsonFile.size();
    if (jsonFileSize > FILE_MAX_SIZE)
    {
        _debug_println(F("json.txt size is too large"));
        jsonFile.close();
        return false;
    }

#ifdef _DEBUG_SPILittleFS_JSON
    _debug_print("json.txt size :");
    _debug_print(jsonFileSize);
    _debug_println(" byte");
    _debug_print(F("Storing : "));
    _debug_print(SerializedJSON);
    _debug_println(F("---end"));
#endif

    jsonFile.println(SerializedJSON); // writting serialized JSON into /json.txt
    jsonFile.close();

    return true;
}

bool FS_jsonRead(String &SerializedJSON)
{
    File jsonFile = LittleFS.open("/json.txt", "r");
    if (!jsonFile)
    {
        _debug_println(F("Failed to open json.txt for reading"));
        jsonFile.close();
        return false;
    }

    size_t jsonFileSize = jsonFile.size();
    if (jsonFileSize > FILE_MAX_SIZE)
    {
        _debug_println(F("json.txt size is too large"));
        jsonFile.close();
        return false;
    }

    SerializedJSON = jsonFile.readString();
    jsonFile.close();

    return true;
}

bool FS_epochWrite(uint32_t epoch, int16_t gmtOffset)
{
    StaticJsonDocument<128> doc;
    doc["epoch"] = epoch;
    doc["GMT"] = gmtOffset;
    String message;
    serializeJsonPretty(doc, message);

    File epochFile = LittleFS.open("/epoch.txt", "w");
    if (!epochFile)
    {
        _debug_println(F("Failed to open epoch.txt for writing"));
        epochFile.close();
        return false;
    }
    size_t epochFileSize = epochFile.size();
    if (epochFileSize > FILE_MAX_SIZE)
    {
        _debug_println(F("epoch.txt size is too large"));
        epochFile.close();
        return false;
    }

#ifdef _DEBUG_SPIFFS_EPOCH
    _debug_print("epoch.txt size :");
    _debug_print(epochFileSize);
    _debug_println(" byte");
    _debug_print(F("Storing : "));
    _debug_print(message);
    _debug_println(F("---end"));
#endif

    epochFile.println(message); // writting epoch into /epoch.txt
    epochFile.close();

    return true;
}

bool FS_epochRead(uint32_t &epoch, int16_t &gmtOffset)
{
    File epochFile = LittleFS.open("/epoch.txt", "r");
    if (!epochFile)
    {
        _debug_println(F("Failed to open epoch.txt for reading"));
        epochFile.close();
        return false;
    }

    size_t epochFileSize = epochFile.size();
    if (epochFileSize > FILE_MAX_SIZE)
    {
        _debug_println(F("epoch.txt size is too large"));
        epochFile.close();
        return false;
    }

    String message = epochFile.readString();
    epochFile.close();

    StaticJsonDocument<128> doc;
    DeserializationError error = deserializeJson(doc, message);
    if (error)
    {
#ifdef _DEBUG_SAVE_RESTORE
        _debug_print(F("deserializeJson() failed: "));
        _debug_println(error.c_str());
#endif
        return false;
    }

    epoch = doc["epoch"];
    gmtOffset = doc["GMT"];

    return true;
}

String FS_command(String data)
{
    String response = "no match";

    if (data.startsWith("deleteConfig"))
    {
        FS_deleteConfig();
        response = "Config deleted";
    }

    if (data.startsWith("deleteWifi"))
    {
        FS_deleteWifi();
        response = "WifiParameters deleted";
    }

    if (data.startsWith("wifi_read"))
    {
        String ssid, pwd;
        FS_wifiRead(ssid, pwd);
        response = ssid + " " + pwd;
    }

    if (data.startsWith("epochInit"))
    {
        response = "failed";
        if (FS_epochWrite(INIT_DATE, 0))
        {
            response = "Success";
        }
    }

    if (response.startsWith("no match"))
    {
        response = "no match";
    }

    return response;
}

String FS_read(String fileName)
{
    File file = LittleFS.open(fileName, "r");
    if (!file)
    {
        _debug_println(F("Failed to open fileName for reading"));
        file.close();
        return "Failed to open fileName for reading";
    }

    return file.readString();
}