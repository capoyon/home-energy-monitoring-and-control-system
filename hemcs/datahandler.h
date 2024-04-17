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



extern RTC_DS3231 rtc;
#define BUFFER_SIZE 512
#define CONFIG_NUM 8

class DataHandler {
private:
    // for automation
    struct Profile {
      uint8_t targetSensor; // Index of the variable to compare to 0 = time, 1 = energy, 2 = power, 3 = voltage, 4 = current
      uint8_t operation;     // 0 for equal, 1 for less than, and 2 for greater than, 3 between
      
      float wantedVal1;
      float wantedVal2; // for between
      
      // for time
      uint8_t month = 0;
      uint8_t monthDay = 0;
      uint8_t weekDay = 0;
      uint8_t hour = 0;
      uint8_t minute = 0;  

      uint8_t taskNum; // task to perform
    };

    // Array to hold profiles
    static const int MAX_PROFILES = 20;
    Profile profiles[MAX_PROFILES];
    int numProfiles = 0;

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
    float energy_last_recorded = 0.0f;
    float power_last_recorded = 0.0f;

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
    char* getAllProfileData();
    void handleSocketCommand(const char *command);
    void handleAutomationCommand(const char* command);

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
    void updateTimeFromNTP();

   void addProfile(uint8_t targetSensor = 0 , uint8_t operation = 0, float wantedVal1 = 0, float wantedVal2 = 0, 
                    uint8_t month = 0, uint8_t monthDay = 0, uint8_t weekDay = 0, uint8_t hour = 0, uint8_t minute = 0,
                    uint8_t taskNum = 0);
    void taskWatcher();
    void task(int taskNum);
    void removeProfile(int indexToRemove);
    void executeTask(int taskNum);
};

#endif //DATAHANDLER_H
