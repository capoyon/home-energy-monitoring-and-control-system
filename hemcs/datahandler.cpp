#include "datahandler.h"

// Function to save configuration to NVS
void DataHandler::saveConfig(const Config& config) {
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

// Function to load configuration from NVS
void DataHandler::loadConfig(Config& config) {
    Preferences preferences;
    preferences.begin("config", true); // Open NVS namespace "config" in read-only mode

    config.wifi_ssid = strdup(preferences.getString("wifi_ssid", "WiFi").c_str());
    config.wifi_password = strdup(preferences.getString("wifi_password", "password112233").c_str());
    config.ap_ssid = strdup(preferences.getString("ap_ssid", "HEMCS").c_str());
    config.ap_password = strdup(preferences.getString("ap_password", "password").c_str());
    config.currency = preferences.getChar("currency", '₱');
    config.electric_rate = preferences.getFloat("electric_rate", 10.0);
    config.is24HourFormat = preferences.getBool("is24HourFormat", false);
    config.isAutoSetTime = preferences.getBool("isAutoSetTime", true);

    preferences.end(); // Close NVS
}

// Constructor definition
DataHandler::DataHandler() {
    Config config; // Creating a config object to pass to loadConfig
    loadConfig(config); // Load configuration upon object creation
}

// Format the sensor data for sending it in overview web
String DataHandler::graphSensorReading(float sensor_data[], size_t data_size) {
    const uint8_t cmd = 2;
    StaticJsonDocument<200> root;
    root["cmd"] = cmd;
    JsonArray arr = root.createNestedArray("data");
    for (size_t i = 0; i < data_size; i++) {
        arr.add(sensor_data[i]);
    }
    String return_val;
    serializeJson(root, return_val);
    return return_val;
}
