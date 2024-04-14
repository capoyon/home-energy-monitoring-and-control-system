#ifndef WEBSOCKET_H
#define WEBSOCKET_H


#include <AsyncWebSocket.h>
#include "datahandler.h"



extern DataHandler datahandler;
extern AsyncWebSocket overview;
extern AsyncWebSocket automate;
extern AsyncWebSocket settings;

void updateOverview();
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len);
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
void initWebSocket();
void cleanupClients();


#endif //WEBSOCKET_H