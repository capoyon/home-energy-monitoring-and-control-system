#ifndef WIFI_H
#define WIFI_H

#include <WiFi.h>

void startWifiAP(const char* ssid, const char* password);
void connectToWifi(const char* ssid, const char* password);


#endif //WIFI_H