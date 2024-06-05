#include "datahandler.h"
#include "html.h"
#include "websocket.h"

unsigned long previousMillis1 = 0;  
unsigned long interval1 = 1000;     
unsigned long previousMillisMinute = 0;  
unsigned long intervalMinute = 60000;     
unsigned long previousMillisHour = 0;  
unsigned long intervalHour = 3600000; 


void setup(){
  Serial.begin(115200);
  datahandler.init();
  pinMode(2, OUTPUT);
  initWebSocket();
  initWebServer();
}


void loop() {
  unsigned long currentMillis = millis();  // Get the current time
  // Timer every seconds
  if (currentMillis - previousMillis1 >= interval1) {
    previousMillis1 = currentMillis;
    if(!isnan(datahandler.readElectricity())){

      if(overview.count()>0) {
        updateOverview();
      }
      // there is 3 Watts change in reading record it to file
      if(abs(datahandler.power - datahandler.power_last_recorded) > 2 ) {
        datahandler.saveSensorReading();
      }
    }
    datahandler.taskWatcher();
  }
  // Timer for 1 minute
  if (currentMillis -  previousMillisMinute >= intervalMinute) {
    previousMillisMinute = currentMillis;
  }
  // save config every hour
  if (currentMillis - previousMillisHour >= intervalHour ) {
    previousMillisHour = currentMillis;
    datahandler.saveSensorReading();
    cleanupClients();
  }
}