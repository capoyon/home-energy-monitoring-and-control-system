#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>


#include "datahandler.h"
#include "wifi.h"
#include "html.h"
#include "websocket.h"

AsyncWebServer server(80); // port for webserver
unsigned long previousMillis = 0;

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

  initWebSocket();
  initWebServer();
}


void loop() {
  unsigned long currentMillis = millis();  

  if (currentMillis - previousMillis >= 1000) {
    cleanupClients();
    updateOverview();
    previousMillis = currentMillis; 
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
