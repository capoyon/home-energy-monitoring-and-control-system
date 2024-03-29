#ifndef WEBSOCKET_H
#define WEBSOCKET_H


#include <AsyncWebSocket.h>

extern AsyncWebSocket ws;

void notifyClients(String data);
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len);
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
void initWebSocket();


#endif //WEBSOCKET_H