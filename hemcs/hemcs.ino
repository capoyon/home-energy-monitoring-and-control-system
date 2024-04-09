#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "datahandler.h"
#include "html.h"
#include "websocket.h"
#include "time.h"

// for time management
const char* ntpServer = "time.google.com";
const long  gmtOffset_sec = 8 * 3600; //philippines gmt
const int   daylightOffset_sec = 0;

// port for webserver
AsyncWebServer server(80); 

// for non blocking delay
unsigned long previousMillis = 0;

void printLocalTime();
void initWebServer();

void setup(){
  Serial.begin(115200);
  datahandler.init();
  
  // Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();
  
  initWebSocket();
  initWebServer();
}


void loop() {
  unsigned long currentMillis = millis();  

  if (currentMillis - previousMillis >= 1000) {
    cleanupClients();
    if(overview.count()>0){
        updateOverview();
    }
    previousMillis = currentMillis;
    // printLocalTime();
  }
}


void initWebServer() {
  //socket connections
  server.addHandler(&overview);
  server.addHandler(&settings);

  // html route for request
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", overview_html, overview_html_len);
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

  server.begin();
}

void printLocalTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  time_t posix_time = mktime(&timeinfo);
  Serial.println(posix_time);
}
