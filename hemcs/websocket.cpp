#include "websocket.h"

DataHandler datahandler;

// Define JSON_DOC_SIZE if not already defined
#define JSON_DOC_SIZE 200

AsyncWebSocket overview("/overview");
AsyncWebSocket settings("/settings");

bool ledState = 0;
const int ledPin = 2;

void updateOverview(float sensor_data[], size_t data_size) {
    const uint8_t cmd = 2;
    StaticJsonDocument<JSON_DOC_SIZE> root; // Use a constant for JSON_DOC_SIZE
    root["cmd"] = cmd;
    JsonArray arr = root.createNestedArray("data");
    for (size_t i = 0; i < data_size; i++) {
        arr.add(sensor_data[i]);
    }
    char* data = (char*)malloc(JSON_DOC_SIZE); // Allocate memory for JSON string
    if (data) {
      serializeJson(root, data, JSON_DOC_SIZE);
      overview.textAll(data);
    } else {
      Serial.println("Error: unable to allocate memory updateOverview()");
    }
    free(data);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
        // Ensure proper termination of the received data
        data[len - 1] = 0;
        Serial.print("Received message: ");
        Serial.println((char *)data);
        const size_t bufferSize = JSON_OBJECT_SIZE(2) + 30;
        DynamicJsonDocument jsonBuffer(bufferSize);

        // Deserialize the JSON data
        DeserializationError error = deserializeJson(jsonBuffer, (char *)data);

        // Check for errors in parsing JSON
        if (error) {
          Serial.print("Failed to parse JSON: ");
          Serial.println(error.c_str());
          return;
        }

        // Extract values from JSON
        if (jsonBuffer.containsKey("cmd") && jsonBuffer["cmd"].is<int>() && jsonBuffer.containsKey("data")) {
            int cmd = jsonBuffer["cmd"];
            JsonObject dataObject = jsonBuffer["data"];
            if (dataObject.containsKey("ssid") && dataObject.containsKey("pass") && dataObject["ssid"].is<const char*>() && dataObject["pass"].is<const char*>()) {
                const char* ssid = dataObject["ssid"];
                const char* password = dataObject["pass"];

                // Do something with the extracted values
                Serial.print("Command: ");
                Serial.println(cmd);
                Serial.print("SSID: ");
                Serial.println(ssid);
                Serial.print("Password: ");
                Serial.println(password);
            } else {
                Serial.println("Invalid data format: ssid and/or pass missing or invalid type");
            }
        } else {
            Serial.println("Invalid data format: cmd missing or invalid type");
        }
    }
}


void onEventOverview(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    switch (type) {
    case WS_EVT_CONNECT:
        Serial.printf("Overview: #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
        break;
    case WS_EVT_DISCONNECT:
        Serial.printf("Overview: #%u disconnected\n", client->id());
        break;
    case WS_EVT_DATA:
        handleWebSocketMessage(arg, data, len);
        break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
        break;
    }
}



void onEventSettings(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
  if(type == WS_EVT_CONNECT){
    //client connected
    Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
    client->printf("%s", datahandler.getSettingsJSON());
  } else if(type == WS_EVT_DISCONNECT){
    //client disconnected
    Serial.printf("ws[%s][%u] disconnect: %u\n", server->url(), client->id());
  } else if(type == WS_EVT_ERROR){
    //error was received from the other end
    Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t*)arg), (char*)data);
  } else if(type == WS_EVT_PONG){
    //pong message was received (in response to a ping request maybe)
    Serial.printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len)?(char*)data:"");
  } else if(type == WS_EVT_DATA){
    //data packet
    AwsFrameInfo * info = (AwsFrameInfo*)arg;
    if(info->final && info->index == 0 && info->len == len){
      //the whole message is in a single frame and we got all of it's data
      Serial.printf("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT)?"text":"binary", info->len);
      if(info->opcode == WS_TEXT){
        data[len] = 0;
        Serial.printf("%s\n", (char*)data);
        datahandler.handleSocketCommand((char*) data);
      } else {
        for(size_t i=0; i < info->len; i++){
          Serial.printf("%02x ", data[i]);
        }
        Serial.printf("\n");
      }
      if(info->opcode == WS_TEXT)
        client->text("I got your text message");
      else
        client->binary("I got your binary message");
    } else {
      //message is comprised of multiple frames or the frame is split into multiple packets
      if(info->index == 0){
        if(info->num == 0)
          Serial.printf("ws[%s][%u] %s-message start\n", server->url(), client->id(), (info->message_opcode == WS_TEXT)?"text":"binary");
        Serial.printf("ws[%s][%u] frame[%u] start[%llu]\n", server->url(), client->id(), info->num, info->len);
      }

      Serial.printf("ws[%s][%u] frame[%u] %s[%llu - %llu]: ", server->url(), client->id(), info->num, (info->message_opcode == WS_TEXT)?"text":"binary", info->index, info->index + len);
      if(info->message_opcode == WS_TEXT){
        data[len] = 0;
        Serial.printf("%s\n", (char*)data);
      } else {
        for(size_t i=0; i < len; i++){
          Serial.printf("%02x ", data[i]);
        }
        Serial.printf("\n");
      }

      if((info->index + len) == info->len){
        Serial.printf("ws[%s][%u] frame[%u] end[%llu]\n", server->url(), client->id(), info->num, info->len);
        if(info->final){
          Serial.printf("ws[%s][%u] %s-message end\n", server->url(), client->id(), (info->message_opcode == WS_TEXT)?"text":"binary");
          if(info->message_opcode == WS_TEXT)
            client->text("I got your text message");
          else
            client->binary("I got your binary message");
        }
      }
    }
  }
}

void initWebSocket() {
    overview.onEvent(onEventOverview);
    settings.onEvent(onEventSettings);
}

void cleanupClients(){
  overview.cleanupClients();
  settings.cleanupClients();
}