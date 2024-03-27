#include "wifi.h"

// Replace with your network credentials
const char* ssid = "WiFi";
const char* password = "password112233";


void initWifi(){
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