#ifndef DATAHANDLER_H
#define DATAHANDLER_H

#include <WiFi.h>
#include <cmath>
// For time
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "RTClib.h"

// Local library
#include "SPIFFS.h"
#include "time.h"
#include "pzem.h"



#define BUFFER_SIZE 512
#define CONFIG_NUM 8

class DataHandler {
private:
    
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

  void reCreateFile(const char *name);



public:
    float energy = 0.0f;
    float power = 0.0f;
    float voltage = 0.0f;
    float current = 0.0f;
    float frequency = 0.0f;
    float powerfactor = 0.0f;

    float prev_energy = 0.0f;
    float prev_power = 0.0f;
    float prev_voltage = 0.0f;
    float prev_current = 0.0f;
    float prev_frequency = 0.0f;
    float prev_powerfactor = 0.0f;

    //Spiffs file directory
    const char* config = "/config.hemcs";
    const char* sensorReading = "/historydata.hemcs";
    const char* automation = "/automation.hemcs";

    void init();
    bool readElectricity();
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

    unsigned long getNTPEpoch();
};

#endif //DATAHANDLER_H
