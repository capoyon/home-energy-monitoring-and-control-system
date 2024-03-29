#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>


#include "datahandler.h"
#include "wifi.h"
#include "html.h"
#include "pzem.h"
#include "websocket.h"

AsyncWebServer server(80);

DataHandler datahandler;

float sensor_data[7];
String socket_data;
Pzem pzem(16,17);




void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);
  Serial.println("");
  Serial.println("");
  Serial.println("");
  
  //connect to wifi and start hotspot
  startWifiAP(datahandler.getAPSsid(), datahandler.getAPPassword());
  connectToWifi(datahandler.getWifiSSID(), datahandler.getWifiPassword());

  initWebSocket();
  server.addHandler(&ws);
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

unsigned long previousMillis = 0;

void loop() {
  unsigned long currentMillis = millis();  

  if (currentMillis - previousMillis >= 1000) {
    // get the sensor data and send it to websocket
    sensor_data[0] = pzem.voltage();
    sensor_data[1] = pzem.current();
    sensor_data[2] = pzem.power();
    sensor_data[3] = pzem.energy();
    sensor_data[4] = pzem.frequency();
    sensor_data[5] = pzem.powerfactor();
    sensor_data[6] = 0.12;
    socket_data = datahandler.graphSensorReading( sensor_data, 7);
    Serial.println(socket_data);
    notifyClients(socket_data);
    previousMillis = currentMillis; 
  }
  
  ws.cleanupClients();
}
