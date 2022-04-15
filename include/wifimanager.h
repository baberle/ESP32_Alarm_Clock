#ifndef __WIFIMANAGER_H
#define __WIFIMANAGER_H

#include <WiFi.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>

/*
    String _hostname;
    String _ssid;
    String _password;
    unsigned long _disconnectTime;
    //void onDisconnect(WiFiEvent_t event, WiFiEventInfo_t info);
    //void onConnect(WiFiEvent_t event, WiFiEvent_t info);
    bool connectSpecific(const char* ssid, const char* password);
    bool connectSpecific(const char* ssid);
    bool connectGeneral(fs::FS &fs);
    public:
    WiFiManager();
    WiFiManager(String hostname);
    void setup();
    void addNetwork(fs::FS &fs, const char* ssid, const char* password);
    unsigned long timeDisconnected();
};*/

void setupWiFi();
void addNetwork(fs::FS &fs, const char* ssid, const char* password);
unsigned long timeDisconnected();
void deliverWebpage();

#endif