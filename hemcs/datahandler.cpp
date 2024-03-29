#include "datahandler.h"
#include <ArduinoJson.h> // Include ArduinoJson.h here if not already included

// Define JSON_DOC_SIZE if not already defined
#define JSON_DOC_SIZE 200

DataHandler::DataHandler() {
    loadConfig(); // Load configuration upon object creation
}

void DataHandler::saveConfig() {
    Preferences preferences;
    preferences.begin("config", false); // Open NVS namespace "config" in read-write mode

    preferences.putString("wifi_ssid", config.wifi_ssid);
    preferences.putString("wifi_password", config.wifi_password);
    preferences.putString("ap_ssid", config.ap_ssid);
    preferences.putString("ap_password", config.ap_password);
    preferences.putChar("currency", config.currency);
    preferences.putFloat("electric_rate", config.electric_rate);
    preferences.putBool("is24HourFormat", config.is24HourFormat);
    preferences.putBool("isAutoSetTime", config.isAutoSetTime);

    preferences.end(); // Close NVS
}

void DataHandler::loadConfig() {
    Preferences preferences;
    preferences.begin("config", true); // Open NVS namespace "config" in read-only mode

    const char* default_wifi_ssid = "WiFi";
    const char* default_wifi_password = "password112233";
    const char* default_ap_ssid = "HEMCS";
    const char* default_ap_password = "password";

    config.wifi_ssid = strdup(preferences.getString("wifi_ssid", default_wifi_ssid).c_str());
    config.wifi_password = strdup(preferences.getString("wifi_password", default_wifi_password).c_str());
    config.ap_ssid = strdup(preferences.getString("ap_ssid", default_ap_ssid).c_str());
    config.ap_password = strdup(preferences.getString("ap_password", default_ap_password).c_str());
    config.currency = preferences.getChar("currency", '₱');
    config.electric_rate = preferences.getFloat("electric_rate", 10.0);
    config.is24HourFormat = preferences.getBool("is24HourFormat", false);
    config.isAutoSetTime = preferences.getBool("isAutoSetTime", true);

    preferences.end(); // Close NVS
}


char* DataHandler::graphSensorReading(float sensor_data[], size_t data_size) {
    const uint8_t cmd = 2;
    StaticJsonDocument<JSON_DOC_SIZE> root; // Use a constant for JSON_DOC_SIZE
    root["cmd"] = cmd;
    JsonArray arr = root.createNestedArray("data");
    for (size_t i = 0; i < data_size; i++) {
        arr.add(sensor_data[i]);
    }
    char* return_val = (char*)malloc(JSON_DOC_SIZE); // Allocate memory for JSON string
    if (return_val) {
        serializeJson(root, return_val, JSON_DOC_SIZE);
    }
    return return_val;
}
