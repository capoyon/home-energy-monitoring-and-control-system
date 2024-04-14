#ifndef DATAHANDLER_H
#define DATAHANDLER_H

#include <WiFi.h>
#include "SPIFFS.h"
#include <ESP32Time.h>
#include "time.h"
#include "pzem.h"

#define BUFFER_SIZE 512
#define CONFIG_NUM 8

class DataHandler {
private:
    // for getting the time for internet
    const char* ntpServer = "time.google.com";
    const long  gmtOffset_sec = 8 * 3600; //philippines gmt
    const int   daylightOffset_sec = 0;
    struct tm timeinfo;
    
    char buffer[BUFFER_SIZE];
    size_t usedStorage;
    size_t totalStorage;
    char wifi_ssid[60]= "WiFi";
    char  wifi_password[60]= "password112233";
    char  ap_ssid[60] = "HEMCS";
    char ap_password[60] = "hemcs123";
    int currency = 0;
    float electric_rate = 18;
    bool is24HourFormat = false;
    bool isAutoSetTime = true;

    void changeAP(const char* name, const char* pass);
    void reCreateFile(const char *name);



public:
    //Spiffs file directory
    const char* config = "/config.hemcs";
    const char* sensorReading = "/historydata.hemcs";
    const char* automation = "/automation.hemcs";

    void init();
    void setCustomTime(const char* posixTime);
    void saveConfig();
    void loadConfig();
    void saveSensorReading();
    void deleteHistoryData();

    char* getSensorDataJSON();
    char* getSettingsJSON();
    void handleSocketCommand(const char *command);

    char* getWifiSSID() {
        return wifi_ssid;
    }
    char* getWifiPassword() {
        return wifi_password;
    }

    char* getAPSsid() {
        return ap_ssid;
    }

    char* getAPPassword() {
        return ap_password;
    }

    void printLocalTime();
};

#endif //DATAHANDLER_H
