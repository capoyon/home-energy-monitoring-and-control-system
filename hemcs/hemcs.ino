#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "datahandler.h"
#include "html.h"
#include "websocket.h"


// port for webserver
AsyncWebServer server(80); 

void initWebServer();

void setup(){
  Serial.begin(115200);
  datahandler.init();
  WiFi.softAP(datahandler.getAPSsid(), datahandler.getAPSsid());
  datahandler.printLocalTime();
  initWebSocket();
  initWebServer();
}


unsigned long previousMillis1 = 0;  // Stores the last time when the timer was updated
unsigned long interval1 = 1000;     // Interval for 1 second

unsigned long previousMillis3 = 0;  // Stores the last time when the timer was updated
unsigned long interval3 = 3000;     // Interval for 3 seconds

unsigned long previousMillisHour = 0;  // Stores the last time when the timer was updated
unsigned long intervalHour = 3600000;     // Hour interval

void loop() {
  unsigned long currentMillis = millis();  // Get the current time
  
  // Timer for 1 second
  if (currentMillis - previousMillis1 >= interval1) {
    previousMillis1 = currentMillis;
    if(overview.count()>0){
        updateOverview();
    }
  }

  // Timer for 3 seconds
  if (currentMillis - previousMillis3 >= interval3) {
    previousMillis3 = currentMillis;
  }

  // save config every hour
  if (currentMillis - previousMillisHour >= intervalHour ) {
    previousMillisHour = currentMillis;
    datahandler.saveSensorReading();
    cleanupClients();
  }
}


void initWebServer() {
  //socket connections
  server.addHandler(&overview);
  server.addHandler(&automate);
  server.addHandler(&settings);

  // html route for request
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", overview_html, overview_html_len);
  });

  server.on("/general.css", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/css", general_css, general_css_len);
  });


  server.on("/overview", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", overview_html, overview_html_len);
  });

  server.on("/history", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", history_html, history_html_len);
  });

  server.on("/automate", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", automate_html, automate_html_len);
  });

  server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", settings_html, settings_html_len);
  });

  server.on(datahandler.sensorReading, HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, datahandler.sensorReading, "text/plain");
  });


  server.begin();
}
