#include "datahandler.h"

Pzem pzem(16,17);

// Private
void DataHandler::changeAP(const char* name, const char* pass) {
    preferences.begin("config", false); // Open NVS namespace "config" in read-write mode

    preferences.putString("ap_ssid", name);
    preferences.putString("ap_password", pass);

    preferences.end(); // Close NVS
    startWifiAP(getAPSsid(), getAPPassword());
    Serial.printf("AP updated, name: %s, password: %s", name, pass);
}

// public

void DataHandler::init() {
  Serial.print("Initializing Preference: ");
  loadConfig(); // Load configuration upon object creation
  Serial.println("Done");
}

void DataHandler::saveConfig() {
    preferences.begin("config", false); // Open NVS namespace "config" in read-write mode

    preferences.putString("wifi_ssid", wifi_ssid);
    preferences.putString("wifi_password", wifi_password);
    preferences.putString("ap_ssid", ap_ssid);
    preferences.putString("ap_password", ap_password);
    preferences.putChar("currency", currency);
    preferences.putFloat("electric_rate", electric_rate);
    preferences.putBool("is24HourFormat", is24HourFormat);
    preferences.putBool("isAutoSetTime", isAutoSetTime);

    preferences.end(); // Close NVS
}

void DataHandler::loadConfig() {
    Preferences preferences;
    preferences.begin("config", true); // Open NVS namespace "config" in read-only mode

    const char* default_wifi_ssid = "WiFi";
    const char* default_wifi_password = "password112233";
    const char* default_ap_ssid = "HEMCS";
    const char* default_ap_password = "password";

    wifi_ssid = strdup(preferences.getString("wifi_ssid", default_wifi_ssid).c_str());
    wifi_password = strdup(preferences.getString("wifi_password", default_wifi_password).c_str());
    ap_ssid = strdup(preferences.getString("ap_ssid", default_ap_ssid).c_str());
    ap_password = strdup(preferences.getString("ap_password", default_ap_password).c_str());
    currency = preferences.getChar("currency", 1);
    electric_rate = preferences.getFloat("electric_rate", 10.0);
    is24HourFormat = preferences.getBool("is24HourFormat", false);
    isAutoSetTime = preferences.getBool("isAutoSetTime", true);

    preferences.end(); // Close NVS
}


char* DataHandler::getSensorDataJSON() {
    float sensor_data[6];
    sensor_data[0] = pzem.energy();
    sensor_data[1] = pzem.power();
    sensor_data[2] = pzem.voltage();
    sensor_data[3] = pzem.current();
    sensor_data[4] = pzem.frequency();
    sensor_data[5] = pzem.powerfactor();
    sprintf(buffer, "{\"data\": [%f, %f, %f, %f, %f, %f]}",
            sensor_data[0], sensor_data[1], sensor_data[2], 
            sensor_data[3], sensor_data[4], sensor_data[5]);
    return buffer;
}

char* DataHandler::getSettingsJSON() {
    sprintf(buffer, "{\"sta\":\"%s\", \"sta_p\":\"%s\", \"ap\":\"%s\",\"ap_p\":\"%s\", \"cur\":\"%d\",\"rate\":%f,\"t_format\":%d,\"t_auto\":%d}",
    wifi_ssid, wifi_password, ap_ssid, ap_password, currency, electric_rate, is24HourFormat, isAutoSetTime);
    return buffer;
}


void DataHandler::handleSocketCommand(const char *command) {
  Serial.printf("Processing: %s", command);
  int opcode;
  char *token;
  char *data1;
  char *data2;

  // Parse opcode
  char opcode_str[3];
  strncpy(opcode_str, command, 2);
  opcode_str[2] = '\0';
  opcode = atoi(opcode_str);

  switch(opcode){
    case 2: //change wifi hotspot
        token = strtok((char *)command + 2, ":=:"); // Skip first two characters (opcode)
        data1 = strdup(token); //name
        token = strtok(NULL, ":=:"); // Get the next token
        data2 = strdup(token); //password
        changeAP(data1, data2);
    break;
    default:
      Serial.println("Error: Invalid OP code");
  }
}