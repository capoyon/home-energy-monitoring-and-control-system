#include "datahandler.h"

Pzem pzem(16, 17);

// Private
void DataHandler::changeAP(const char* name, const char* pass) {
  // chagne the ap name and password and its save config files.
}


void DataHandler::init() {
  SPIFFS.begin(true);
  freeSPIFFS = (SPIFFS.totalBytes() - SPIFFS.usedBytes()) / (1024.0 * 1024.0);
  Serial.printf("Available SPIFFS: %f MB\n", freeSPIFFS);
  loadConfig();

  //connect to wifi and start hotspot
  WiFi.softAP(ap_ssid, ap_password);
  WiFi.begin(wifi_ssid, wifi_password);
}

void DataHandler::reCreateFile(const char* name) {
  SPIFFS.remove(name);
  File file = SPIFFS.open(name, FILE_WRITE);
  file.close();
}


void DataHandler::saveConfig() {
  reCreateFile(config);
  File file = SPIFFS.open(config, FILE_WRITE);
  sprintf(buffer, ":=:%s:=:%s:=:%s:=:%s:=:%d:=:%f:=:%d:=:%d:=:",
          wifi_ssid, wifi_password, ap_ssid, ap_password, currency, electric_rate, is24HourFormat, isAutoSetTime);
  file.write((uint8_t*)buffer, strlen(buffer));
}

void DataHandler::loadConfig() {

  //Read the config from SPIFFS
  File file = SPIFFS.open(config, "r");
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }
  file.readStringUntil('\n').toCharArray(buffer, BUFFER_SIZE);   // Read the first line of the file
  file.close();

  if(strlen(buffer)<1){
    Serial.println("No config data");
    return;
  }


  Serial.printf("Config Data: %s\n", buffer);
  char *token;
  char data[8][64];

  // Parsing the string using strtok function
  token = strtok(buffer, ":=:"); // Delimiter is ":=:"
  int i = 0;
  while (token != NULL && i < CONFIG_NUM) {
      strcpy(data[i], token);
      token = strtok(NULL, ":=:");
      i++;
  }
  Serial.printf("Saved config data: %s\n", getSettingsJSON());
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

char* DataHandler::getSettingsJSON()
{
  char localIP[16];
  if (WiFi.status() == WL_CONNECTED) {
    strcpy(localIP, WiFi.localIP().toString().c_str());
  } else {
    strcpy(localIP, "NOT CONNECTED");
  }
  sprintf(buffer, "{\"sta\":\"%s\", \"sta_p\":\"%s\", \"ap\":\"%s\",\"ap_p\":\"%s\", \"cur\":\"%d\",\"rate\":%f,\"t_format\":%d,\"t_auto\":%d}",
          wifi_ssid, localIP, ap_ssid, ap_password, currency, electric_rate, is24HourFormat, isAutoSetTime);
  return buffer;
}


/*
 This is for parsing the socket message from settings page.
    Expected input: ":=:42:=:data 1:=:data 2:=:"
    :=: - the data separator/delimiter.
    42 - first 2 digit are the operation code
    data 1 and 2 - the data for the operation
*/
void DataHandler::handleSocketCommand(const char* command) {
  char* token;
  size_t command_len = strlen(command) + 1;
  if (command_len > 190) {
    Serial.println("Socket data to large");
    return;
  }
  char str[command_len]; 
  strcpy(str, command);
  char data[3][64];
  token = strtok(str, ":=:"); 
  int i = 0;
  while (token != NULL && i < 3) {
    strcpy(data[i], token);
    token = strtok(NULL, ":=:");
    i++;
  }

  int cmd = atoi(data[0]);
  switch (cmd) {
    case 40: // Change Wi-Fi station credential
      strcpy(wifi_ssid, data[1]);
      strcpy(wifi_password, data[2]);
      Serial.printf("WiFi credentials have changed: %s:%s\n", wifi_ssid, wifi_password);
      WiFi.begin(wifi_ssid, wifi_password);
      break;
    case 41: // Change Wi-Fi hotspot credential
      strcpy(ap_ssid, data[1]);
      strcpy(ap_password, data[2]);
      Serial.printf("AP credentials have changed: %s:%s\n", ap_ssid, ap_password);
      WiFi.softAP(ap_ssid, ap_password);
      break;
    case 42: // change currency type
      currency =  atoi(data[1]);
      Serial.printf("Currency set to: %d\n", currency);
      break;
    case 43: // change electricity rate
      electric_rate = atof(data[1]);
      Serial.printf("Electricity rate is set to: %f\n", electric_rate);
      break;
    case 44: // is 24 hour format?
      is24HourFormat = atoi(data[1]);
      Serial.printf("24 hour format is set to: %d\n", is24HourFormat);
      break;
    case 45: // is auto set time
      isAutoSetTime = atoi(data[1]);
      Serial.printf("Autotime is set to: %d\n", isAutoSetTime);
      break;
    case 46: // set time
      // Todo
      Serial.printf("Time is set to: %s\n", data[1]);
      return;
      break;
    case 99:
      ESP.restart();
    default:
      Serial.println("Error: invalid command from websocket");
      return;
  }
  saveConfig();
}