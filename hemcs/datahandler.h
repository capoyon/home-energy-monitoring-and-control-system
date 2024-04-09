#ifndef DATAHANDLER_H
#define DATAHANDLER_H

#include "SPIFFS.h"

#include "wifi.h"
#include "pzem.h"

#define BUFFER_SIZE 512
#define CONFIG_NUM 8

class DataHandler {
private:
    
    char buffer[BUFFER_SIZE];
    

    const char* config = "/config.json";
    const char* data = "/data.json";

    float freeSPIFFS;
    char wifi_ssid[60]= "WiFi";
    char  wifi_password[60]= "password112233";
    char  ap_ssid[60] = "HEMCS";
    char ap_password[60] = "hemcs123";
    char currency = 'P';
    float electric_rate = 18;
    bool is24HourFormat = false;
    bool isAutoSetTime = true;

    void changeAP(const char* name, const char* pass);
    void reCreateFile(const char *name);

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
