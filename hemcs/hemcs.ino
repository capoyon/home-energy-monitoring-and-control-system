#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>


#include "datahandler.h"
#include "wifi.h"
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
  // Serial port for debugging purposes
  Serial.begin(115200);
  Serial.println("");
  Serial.println("");
  Serial.println("");
  datahandler.init();
  //connect to wifi and start hotspot
  startWifiAP(datahandler.getAPSsid(), datahandler.getAPPassword());
  connectToWifi(datahandler.getWifiSSID(), datahandler.getWifiPassword());
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
    updateOverview();
    previousMillis = currentMillis;
    printLocalTime();
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

    // Print the POSIX time
  Serial.println(posix_time);
  //Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  // Serial.print("Day of week: ");
  // Serial.println(&timeinfo, "%A");
  // Serial.print("Month: ");
  // Serial.println(&timeinfo, "%B");
  // Serial.print("Day of Month: ");
  // Serial.println(&timeinfo, "%d");
  // Serial.print("Year: ");
  // Serial.println(&timeinfo, "%Y");
  // Serial.print("Hour: ");
  // Serial.println(&timeinfo, "%H");
  // Serial.print("Hour (12 hour format): ");
  // Serial.println(&timeinfo, "%I");
  // Serial.print("Minute: ");
  // Serial.println(&timeinfo, "%M");
  // Serial.print("Second: ");
  // Serial.println(&timeinfo, "%S");

  // Serial.println("Time variables");
  // char timeHour[3];
  // strftime(timeHour,3, "%H", &timeinfo);
  // Serial.println(timeHour);
  // char timeWeekDay[10];
  // strftime(timeWeekDay,10, "%A", &timeinfo);
  // Serial.println(timeWeekDay);
  // Serial.println();
}
