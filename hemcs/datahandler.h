#ifndef DATAHANDLER_H
#define DATAHANDLER_H

#include <ArduinoJson.h>
#include <Preferences.h>

class DataHandler {
private:
    struct Config {
        char* wifi_ssid;
        char* wifi_password;
        char* ap_ssid;
        char* ap_password;
        char currency;  // Changed to char from string
        float electric_rate;
        bool is24HourFormat;
        bool isAutoSetTime;
    };

    Config config;

public:
    DataHandler(); // Constructor declaration
    void loadConfig(Config& config);
    void saveConfig(const Config& config);
    String graphSensorReading(float sensor_data[], size_t data_size);

    char* getWifiSSID() {
        return config.wifi_ssid;
    }

    char* getWifiPassword() {
        return config.wifi_password;
    }

    char* getAPSsid() {
        return config.ap_ssid;
    }

    char* getAPPassword() {
        return config.ap_password;
    }

    char getCurrency() {
        return config.currency;
    }

    float getElectricRate() {
        return config.electric_rate;
    }

    bool getIs24HourFormat() {
        return config.is24HourFormat;
    }

    bool getIsAutoSetTime() {
        return config.isAutoSetTime;
    }

};

#endif //DATAHANDLER_H
