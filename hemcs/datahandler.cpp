#include "datahandler.h"


// pin pzem power meter
Pzem pzem(16, 17);

//Time management
RTC_DS3231 rtc;
const char* ntpServer = "time.facebook.com";
const int  gmtOffset_sec = 8 * 3600; //philippines gmt
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServer, gmtOffset_sec);


/*
  * start rtc
  * start spiffs
  * load the save config from spiffs to a into variable
  * start connectitvity
*/
void DataHandler::init() {
  SPIFFS.begin(true);
  loadConfig();

  //start soft ap
  Serial.printf("\nStarting SoftAP: %s, passwd: %s\n", ap_ssid, ap_password);
  WiFi.softAP(ap_ssid, ap_password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  // connect to wifi
  int timeout = 100; // 1 second delay check
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
  } else {
    Serial.print("Wifi connecting timout");
  }


  if(rtc.begin()) {
    updateTimeFromNTP();
    DateTime now = rtc.now();
    Serial.print("RTC timecheck: ");
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
    case 47:// delete history data
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

void DataHandler::handleAutomationCommand(const char* command) {
  char* token;
  size_t command_len = strlen(command) + 1;
  if (command_len > 190) {
    Serial.println("Socket data to large");
    return;
  }
  char str[command_len]; 
  strcpy(str, command);
  char data[10][20];
  token = strtok(str, ":"); 
  int i = 0;
  while (token != NULL && i < 10) {
    strcpy(data[i], token);
    token = strtok(NULL, ":");
    i++;
  }

  uint8_t targetSensor = atoi(data[0]);
  uint8_t operation = atoi(data[1]);
  
  float wantedVal1 = atof(data[2]);
  float wantedVal2 = atof(data[3]);
  // for time
  uint8_t month = atoi(data[4]);
  uint8_t monthDay = atoi(data[5]);
  uint8_t weekDay = atoi(data[6]);
  uint8_t hour = atoi(data[7]);
  uint8_t minute  = atoi(data[8]);

  uint8_t taskNum = atoi(data[9]);
  Serial.printf("Saving Task: %d::%d::%0.2f::%0.2f::%d::%d::%d::%d::%d::%d", targetSensor, operation, wantedVal1, wantedVal2,
              month, monthDay, weekDay, hour, minute, taskNum);
  if(targetSensor == 99) {
    removeProfile(operation);
  } else {
    addProfile(targetSensor, operation, wantedVal1, wantedVal2,
              month, monthDay, weekDay, hour, minute, taskNum);
  }
  
}

char* DataHandler::getAllProfileData() {
  Serial.print("Number of profiles: ");
  Serial.println(numProfiles);
  buffer[0] = '\0';
  strcat(buffer, "[");

    // Concatenate each iteration to the buffer
    for (int i = 0; i < numProfiles; i++) {
        char temp[50]; // Adjust size as needed
        sprintf(temp, "[%d,%d,%0.2f,%0.2f,%d,%d,%d,%d,%d,%d]", profiles[i].targetSensor, profiles[i].operation, profiles[i].wantedVal1,
            profiles[i].wantedVal2, profiles[i].month, profiles[i].monthDay, profiles[i].weekDay,
            profiles[i].hour, profiles[i].minute, profiles[i].taskNum);
        
        // Concatenate temp to buffer
        strcat(buffer, temp);

        // Add '&' after each iteration except the last one
        if (i != numProfiles - 1) {
            strcat(buffer, ",");
        }
    }
   strcat(buffer, "]");
  return buffer;
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


/// automation

void DataHandler::addProfile(uint8_t targetSensor, uint8_t operation, float wantedVal1, float wantedVal2,
                             uint8_t month, uint8_t monthDay, uint8_t weekDay, uint8_t hour, uint8_t minute, 
                             uint8_t taskNum){
    if (numProfiles < MAX_PROFILES) {
        profiles[numProfiles].targetSensor = targetSensor;
        profiles[numProfiles].operation = operation;
        profiles[numProfiles].month = month;
        profiles[numProfiles].monthDay = monthDay;
        profiles[numProfiles].weekDay = weekDay;
        profiles[numProfiles].hour = hour;
        profiles[numProfiles].minute = minute;
        profiles[numProfiles].wantedVal1 = wantedVal1;
        profiles[numProfiles].wantedVal2 = wantedVal2;
        profiles[numProfiles].taskNum = taskNum;
        numProfiles++;
    } else {
        Serial.printf("Max profiles reached.\n");
    }
}

void DataHandler::taskWatcher() {
  for (int i = 0; i < numProfiles; i++) {
    bool conditionMet = false;
    float targetSensorVal = 0.0f;

    if (profiles[i].targetSensor == 0 ) {
      // Time-based condition check
      DateTime now = rtc.now();
      uint8_t month = now.month();
      uint8_t monthDay = now.day();
      uint8_t weekDay = now.dayOfTheWeek();
      uint8_t hour = now.hour();
      uint8_t minute = now.minute();
      if ((profiles[i].month == 0 || profiles[i].month == month) &&
          (profiles[i].monthDay == 0 || profiles[i].monthDay == monthDay) &&
          (profiles[i].weekDay == 0 || profiles[i].weekDay == weekDay) &&
          (profiles[i].hour == hour) &&
          (profiles[i].minute == minute)) {
        conditionMet = true;
      }
    }
    else {
      // Sensor-based condition check
      switch (profiles[i].targetSensor) {
        case 1:
          targetSensorVal = energy;
          break;
        case 2:
          targetSensorVal = power;
          break; // Add break statement
        case 3:
          targetSensorVal = voltage;
          break;
        case 4:
          targetSensorVal = current;
          break;
        default:
          Serial.printf("Invalid target sensor in profile.\n");
          continue; // Skip this profile
      }
      
      // Operation check
      switch (profiles[i].operation) {
        case 0: // Equal
          conditionMet = (round(targetSensorVal * 100) == round(profiles[i].wantedVal1 * 100));
          break;
        case 1: // Less than
          conditionMet = (round(targetSensorVal * 100) < round(profiles[i].wantedVal1 * 100));
          break;
        case 2: // Greater than
          conditionMet = (round(targetSensorVal * 100) > round(profiles[i].wantedVal1 * 100));
          break;
        case 3: // Range
          conditionMet = (round(targetSensorVal * 100) > round(profiles[i].wantedVal1 * 100) && round(targetSensorVal * 100) < round(profiles[i].wantedVal2 * 100));
          break;
        default:
          Serial.printf("Invalid operation in profile.\n");
          continue; // Skip this profile
      }

    }
    
    if (conditionMet) {
      // Run the corresponding task
      executeTask(profiles[i].taskNum);
      removeProfile(i);
    }
  }
}


void DataHandler::executeTask(int taskNum) {
  switch (taskNum) {
      case 1:
          Serial.printf("Running Task 1\n");
          digitalWrite(2, 0);
          break;
      case 2:
          Serial.printf("Running Task 2\n");
          digitalWrite(2, 1);
          break;
      case 3:
          Serial.printf("Running Task 2\n");
          digitalWrite(2, !digitalRead(2));
          break;
      // Add more cases for additional tasks if needed
      default:
          Serial.printf("Invalid task number.\n");
          break;
  }
}

void DataHandler::removeProfile(int indexToRemove) {
  if (indexToRemove < 0 || indexToRemove >= numProfiles) {
      // Invalid index
      return;
  }

  // Shift elements after the indexToRemove one position to the left
  for (int i = indexToRemove; i < numProfiles - 1; ++i) {
      profiles[i] = profiles[i + 1];
  }

  // Decrement the count of profiles
  --numProfiles;
}
