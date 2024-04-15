#include "datahandler.h"
// pin pzem power meter
Pzem pzem(16, 17);

ESP32Time rtc(8 * 3600); 

// Private
void DataHandler::changeAP(const char* name, const char* pass) {
  // chagne the ap name and password and its save config files.
}


void DataHandler::init() {
  SPIFFS.begin(true);
  loadConfig();

  //start soft ap
  Serial.printf("Starting SoftAP: %s, passwd: %s\n", ap_ssid, ap_password);
  WiFi.softAP(ap_ssid, ap_password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  // connect to wifi
  WiFi.begin(wifi_ssid, wifi_password);

  // Get the time from net and save it locally
  // rtc.setTime(30, 24, 15, 17, 1, 2021);  // 17th Jan 2021 15:24:30
  rtc.setTime(1609459200);  // 1st Jan 2021 00:00:00
  // //rtc.offset = 7200; // change offset value
  printLocalTime();
  setTimeFromNTP();
  printLocalTime();
}


void DataHandler::reCreateFile(const char* name) {
  SPIFFS.remove(name);
  File file = SPIFFS.open(name, FILE_WRITE);
  file.close();
}


void DataHandler::saveConfig() {;
  File file = SPIFFS.open(config, FILE_WRITE);
  sprintf(buffer, ":=:%s:=:%s:=:%s:=:%s:=:%d:=:%f:=:%d:=:%d:=:",
          wifi_ssid, wifi_password, ap_ssid, ap_password, currency, electric_rate, is24HourFormat, isAutoSetTime);
  file.write((uint8_t*)buffer, strlen(buffer));
  file.close();
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
  strcpy(wifi_ssid, data[0]);
  strcpy(wifi_password, data[1]);
  strcpy(ap_ssid, data[2]);
  strcpy(ap_password, data[3]);
  currency = atoi(data[4]);
  electric_rate = atof(data[5]);
  is24HourFormat = atoi(data[6]);
  isAutoSetTime = atoi(data[7]);
  Serial.printf("config loaded: %s\n", getSettingsJSON());
}

void DataHandler::saveSensorReading() {
  File file = SPIFFS.open(sensorReading, FILE_APPEND);
  sprintf(buffer, ":=:%s:=:%s:=:%s:=:%s:=:%d:=:%f:=:%d:=:%d:=:\n",
          wifi_ssid, wifi_password, ap_ssid, ap_password, currency, electric_rate, is24HourFormat, isAutoSetTime);
  file.write((uint8_t*)buffer, strlen(buffer));
  file.close();
}

void DataHandler::deleteHistoryData() {
  reCreateFile(sensorReading);
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
  totalStorage =  SPIFFS.totalBytes();
  usedStorage =  SPIFFS.usedBytes();
  sprintf(buffer, "{\"sta\":\"%s\", \"sta_p\":\"%s\", \"ap\":\"%s\",\"ap_p\":\"%s\", \"cur\":\"%d\",\"rate\":%f,\"t_format\":%d,\"t_auto\":%d,\"total_storage\":%d,\"used_storage\":%d}",
          wifi_ssid, localIP, ap_ssid, ap_password, currency, electric_rate, is24HourFormat, isAutoSetTime, totalStorage, usedStorage);
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
      WiFi.begin(wifi_ssid, wifi_password);
      Serial.printf("WiFi credentials have changed: %s:%s\n", wifi_ssid, wifi_password);
      break;
    case 41: // Change Wi-Fi hotspot credential
      strcpy(ap_ssid, data[1]);
      strcpy(ap_password, data[2]);
      WiFi.softAP(ap_ssid, ap_password);
      Serial.printf("AP credentials have changed: %s:%s\n", ap_ssid, ap_password);
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
    case 47: // delete history data
      // Todo
      deleteHistoryData();
      Serial.printf("History data deleted\n");
      return;
      break;
    case 98:
      pzem.reset();
      break;
    case 99:
      ESP.restart();
    default:
      Serial.println("Error: invalid command from websocket");
      return;
  }
  saveConfig();
}


/**** Time Stuffs ****/

void DataHandler::setTimeFromNTP() {
  configTime(gmtOffset_sec, daylightOffset_sec, ntp1, ntp2, ntp3);
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)){
    Serial.print("NTP Time: ");
    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S"); 
    rtc.setTimeStruct(timeinfo); 
  }
}

void DataHandler::setCustomTime(const char* posixTime) {
  time_t rawtime;
  struct tm timeinfo;
  
  strptime(posixTime, "%Y-%m-%dT%H:%M:%S", &timeinfo);
  rawtime = mktime(&timeinfo);
  localtime(&rawtime);
}

void DataHandler::printLocalTime(){
  Serial.println(rtc.getTimeDate(true));
}