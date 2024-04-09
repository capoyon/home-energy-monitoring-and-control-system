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
  char data[8][64]; // Assuming maximum length of each data is 20 characters

  // Parsing the string using strtok function
  token = strtok(buffer, ":=:"); // Delimiter is ":=:"
  int i = 0;
  while (token != NULL && i < CONFIG_NUM) {
      strcpy(data[i], token);
      token = strtok(NULL, ":=:");
      i++;
  }

  for(i = 0; i < 8; i++){
    Serial.println(data[i]);
  }
  strcpy(wifi_ssid, data[0]);
  strcpy(wifi_password, data[1]);
  strcpy(ap_ssid, data[2]);
  strcpy(ap_password, data[3]);
  currency = data[4][0];
  electric_rate = atof(data[5]);
  is24HourFormat = atoi(data[6]);
  isAutoSetTime = atoi(data[7]);
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

void DataHandler::handleSocketCommand(const char* command) {
  char* token;
  size_t command_len = strlen(command) + 1;
  if (command_len > 190) {
    Serial.println("Socket data to large");
    return;
  }
  char str[command_len];  // Make a copy of the command string
  strcpy(str, command);
  char data[3][64];  // Assuming maximum length of each data is 20 characters

  // Parsing the string using strtok function
  token = strtok(str, ":=:");  // Delimiter is ":=:"
  int i = 0;
  while (token != NULL && i < 3) {
    strcpy(data[i], token);
    token = strtok(NULL, ":=:");
    i++;
  }

  int cmd = atoi(data[0]);
  switch (cmd) {
    case 40:
      strcpy(wifi_ssid, data[1]);
      strcpy(wifi_password, data[2]);
      Serial.printf("WiFi credentials have changed: %s:%s", data[1], data[2]);
      connectToWifi(wifi_ssid, wifi_password);
      break;
    case 41:
      strcpy(ap_ssid, data[1]);
      strcpy(ap_password, data[2]);
      Serial.printf("AP credentials have changed: %s:%s", data[1], data[2]);
      startWifiAP(ap_ssid, ap_password);
      break;
    default:
      Serial.println("Error: invalid command from websocket");
      return;
  }
  saveConfig();
}