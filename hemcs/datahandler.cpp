#include "datahandler.h"
// pin pzem power meter
Pzem pzem(16, 17);

//Time management
RTC_DS3231 rtc;
const char* ntpServer = "time.facebook.com";
const int  gmtOffset_sec = 8 * 3600; //philippines gmt

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServer, gmtOffset_sec);



void DataHandler::init() {
  SPIFFS.begin(true);
  loadConfig();

  // connect to wifi
  int timeout = 30; //3 seconds
  Serial.printf("Conecting WiFi: %s, passwd: %s ", wifi_ssid, wifi_password);
  WiFi.begin(wifi_ssid, wifi_password);
  while(WiFi.status() != WL_CONNECTED){
    if(timeout == 0 ) {
      Serial.printf("\nUnable to connect WiFi\n");
      break;
    }
    Serial.print(".");
    timeout--;
    delay(100);
  }
  if((WiFi.status() == WL_CONNECTED)){
    Serial.print("\nWifi Connected with Local IP: ");
    Serial.println(WiFi.localIP());
  }
  //start soft ap
  Serial.printf("\nStarting SoftAP: %s, passwd: %s\n", ap_ssid, ap_password);
  WiFi.softAP(ap_ssid, ap_password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  if(rtc.begin()) {
    updateTimeFromNTP();
    DateTime now = rtc.now();
    Serial.print("ESP32 RTC Date Time: ");
    Serial.printf("RTC epoch: %lu = ", now.unixtime());
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(" (");
    Serial.print(now.dayOfTheWeek());
    Serial.print(") ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.println(now.second(), DEC);
  } else {
    Serial.println("RTC module is NOT found");
  }
}

bool DataHandler::readElectricity() {
  if(isnan(pzem.energy())) return false;
  prev_energy = energy;
  prev_power = power;
  prev_voltage = voltage;
  prev_current = current;
  prev_frequency = frequency;
  prev_powerfactor = powerfactor;

  energy = pzem.energy();
  power = pzem.power();
  voltage = pzem.voltage();
  current = pzem.current();
  frequency = pzem.frequency();
  powerfactor = pzem.powerfactor();
  return true;
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
  sprintf(buffer, "%lu:%.2f\n", rtc.now().unixtime(), power);
  file.write((uint8_t*)buffer, strlen(buffer));
  file.close();
  power_last_recorded = power;
}

void DataHandler::deleteHistoryData() {
  reCreateFile(sensorReading);
}


char* DataHandler::getSensorDataJSON() {
  sprintf(buffer, "{\"data\": [%f, %f, %f, %f, %f, %f]}", energy, power, voltage, current, frequency, powerfactor);
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
unsigned long DataHandler::getNTPEpoch() {
  timeClient.update();
  return timeClient.getEpochTime();
}

void DataHandler::updateTimeFromNTP(){
  if(isAutoSetTime && timeClient.update()) {
      unsigned long epochTime = timeClient.getEpochTime();
      rtc.adjust(DateTime(epochTime));
      Serial.print("Time updated from NTP: ");
      Serial.println(epochTime);
    }
}
