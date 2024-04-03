#include "websocket.h"

DataHandler datahandler;

AsyncWebSocket overview("/overview");
AsyncWebSocket settings("/settings");

bool ledState = 0;
const int ledPin = 2;

void updateOverview() {
    overview.textAll(datahandler.getSensorDataJSON());
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



void onEventSettings(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len) {
    static const size_t BUFFER_SIZE = 256;
    char buffer[BUFFER_SIZE];

    if (type == WS_EVT_CONNECT) {
        //client connected
        Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
        client->printf("%s", datahandler.getSettingsJSON());
    } else if (type == WS_EVT_DISCONNECT) {
        //client disconnected
        Serial.printf("ws[%s][%u] disconnect: %u\n", server->url(), client->id());
    } else if (type == WS_EVT_ERROR) {
        //error was received from the other end
        Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t*)arg), (char*)data);
    } else if (type == WS_EVT_PONG) {
        //pong message was received (in response to a ping request maybe)
        Serial.printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len) ? (char*)data : "");
    } else if (type == WS_EVT_DATA) {
        //data packet
        AwsFrameInfo * info = (AwsFrameInfo*)arg;
        if (info->final && info->index == 0 && info->len == len) {
            //the whole message is in a single frame and we got all of it's data
            Serial.printf("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT) ? "text" : "binary", info->len);
            if (info->opcode == WS_TEXT) {
                strncpy(buffer, (char*)data, min(len, BUFFER_SIZE - 1)); // Copy at most BUFFER_SIZE - 1 characters to buffer
                buffer[min(len, BUFFER_SIZE - 1)] = '\0'; // Null-terminate the string
                Serial.printf("%s\n", buffer);
                datahandler.handleSocketCommand(buffer);
            } else {
                for (size_t i = 0; i < info->len; i++) {
                    Serial.printf("%02x ", data[i]);
                }
                Serial.printf("\n");
            }
            if (info->opcode == WS_TEXT)
                client->text("I got your text message");
            else
                client->binary("I got your binary message");
        } else {
            // Handle multi-frame messages here
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