#include "websocket.h"

DataHandler datahandler;

AsyncWebSocket overview("/overview");
AsyncWebSocket automate("/automate");
AsyncWebSocket settings("/settings");


const size_t socket_buffer_size = 256;
char socket_buffer[socket_buffer_size];

const size_t automation_buffer_size = 256;
char automation_buffer[automation_buffer_size];

void updateOverview() {
    overview.textAll(datahandler.getSensorDataJSON());
}

void onEventOverview(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    switch (type) {
    case WS_EVT_CONNECT:
        Serial.printf("Overview: #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
        client->printf("%s", datahandler.getSettingsJSON());
        break;
    case WS_EVT_DISCONNECT:
        Serial.printf("Overview: #%u disconnected\n", client->id());
        break;
    }
}

void onEventAutomate(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_CONNECT) {
        //client connected
        Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
        client->printf("%s", datahandler.getAllProfileData());
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
                strncpy(automation_buffer, (char*)data, min(len, automation_buffer_size - 1)); // Copy at most BUFFER_SIZE - 1 characters to buffer
                automation_buffer[min(len, automation_buffer_size - 1)] = '\0'; // Null-terminate the string
                Serial.printf("%s\n", automation_buffer);
                datahandler.handleAutomationCommand(automation_buffer);
                automate.textAll(datahandler.getAllProfileData());
            } else {
                for (size_t i = 0; i < info->len; i++) {
                    Serial.printf("%02x ", data[i]);
                }
                Serial.printf("\n");
            }
            // if (info->opcode == WS_TEXT)
            //     client->text("I got your text message");
            // else
            //     client->binary("I got your binary message");
        } else {
            // Handle multi-frame messages here
        }
    }
}

void onEventSettings(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len) {
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
                strncpy(socket_buffer, (char*)data, min(len, socket_buffer_size - 1)); // Copy at most BUFFER_SIZE - 1 characters to buffer
                socket_buffer[min(len, socket_buffer_size - 1)] = '\0'; // Null-terminate the string
                Serial.printf("%s\n", socket_buffer);
                datahandler.handleSocketCommand(socket_buffer);
                settings.textAll(datahandler.getSettingsJSON());
                overview.textAll(datahandler.getSettingsJSON());
            } else {
                for (size_t i = 0; i < info->len; i++) {
                    Serial.printf("%02x ", data[i]);
                }
                Serial.printf("\n");
            }
            // if (info->opcode == WS_TEXT)
            //     client->text("I got your text message");
            // else
            //     client->binary("I got your binary message");
        } else {
            // Handle multi-frame messages here
        }
    }
}

void initWebSocket() {
    overview.onEvent(onEventOverview);
    automate.onEvent(onEventAutomate);
    settings.onEvent(onEventSettings);
}

void cleanupClients(){
  overview.cleanupClients();
  automate.cleanupClients();
  settings.cleanupClients();
}