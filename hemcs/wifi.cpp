#include "wifi.h"


void startWifiAP(const char* ssid, const char* password) {
  WiFi.softAP(ssid, password);
  Serial.print("WiFi AP started with SSID: ");
  Serial.print(ssid);
  Serial.print(", ");
  Serial.print("password: ");
  Serial.println(password);
}

void connectToWifi(const char* ssid, const char* password) {
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to SSID: ");
  Serial.println(ssid);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.print("\nConnected to SSID: ");
  Serial.println(ssid);
  Serial.print("Local IP: ");
  // Print ESP Local IP Address
  Serial.println(WiFi.localIP());
}

void initWifi(){
  startWifiAP("HEMCS", "hemcs123");
  connectToWifi("WiFi", "password112233");
}