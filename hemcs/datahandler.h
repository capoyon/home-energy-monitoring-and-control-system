#ifndef DATAHANDLER_H
#define DATAHANDLER_H

#include <Preferences.h>
#include <ArduinoJson.h>
#include "wifi.h"
#include "pzem.h"



class DataHandler {
private:

    Preferences preferences;

    char* wifi_ssid;
    char* wifi_password;
    char* ap_ssid;
    char* ap_password;
    uint8_t currency;
    float electric_rate;
    bool is24HourFormat;
    bool isAutoSetTime;


    // for automation
    

    char buffer[512];

    void changeAP(const char* name, const char* pass);

public:
    void init();
    void saveConfig();
    void loadConfig();
    char* getWifiSSID() {
        return wifi_ssid;
    }


    char* getSensorDataJSON();
    char* getSettingsJSON();


    void handleSocketCommand(const char *command);

    char* getWifiPassword() {
        return wifi_password;
    }

    char* getAPSsid() {
        return ap_ssid;
    }

    char* getAPPassword() {
        return ap_password;
    }

    char getCurrency() {
        return currency;
    }

    float getElectricRate() {
        return electric_rate;
    }
    bool getIs24HourFormat() {
        return is24HourFormat;
    }

    bool getIsAutoSetTime() {
        return isAutoSetTime;
    }
};

#endif //DATAHANDLER_H
