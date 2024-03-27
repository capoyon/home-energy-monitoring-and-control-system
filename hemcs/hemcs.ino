#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "wifi.h"
#include "html.h"
#include "datahandler.h"


bool ledState = 0;
const int ledPin = 2;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");


void notifyClients() {
 // Sample sensor data
    double sensor_data[] = {1.34, 32.43, 2.23};
    size_t data_size = 3;

    // Get JSON object from sensor data
    StaticJsonDocument<200> jsonDoc = graphSensorReading(sensor_data, data_size);

    // Convert JSON object to string
    String jsonString;
    serializeJson(jsonDoc, jsonString);

    // Send JSON string over WebSocket
    ws.textAll(jsonString);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    if (strcmp((char*)data, "toggle") == 0) {
      ledState = !ledState;
      notifyClients();
    }
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

String processor(const String& var){
  Serial.println(var);
  if(var == "STATE"){
    if (ledState){
      return "ON";
    }
    else{
      return "OFF";
    }
  }
  return String();
}

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  
  initWifi();

  initWebSocket();

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

  // Start server
  server.begin();
}


unsigned long previousMillis = 0; // for non-blocking delay

void loop() {
  unsigned long currentMillis = millis();  // Get the current time

  if (currentMillis - previousMillis >= 1000) { //one second non-blocking delay
    Serial.println("1 second");
    notifyClients();
    
    previousMillis = currentMillis; 
  }
  ws.cleanupClients();
  digitalWrite(ledPin, ledState);
}
