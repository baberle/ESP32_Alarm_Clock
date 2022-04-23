#ifndef __WIFIMANAGER_H
#define __WIFIMANAGER_H

#include <WiFi.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include <string.h>

void setupWiFi();
void addNetwork(const char* ssid, const char* password);
unsigned long timeDisconnected();
void deliverWebpage();
bool inApMode();
String getIpAddr();

#endif