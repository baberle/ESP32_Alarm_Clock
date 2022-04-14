#ifndef __WIFIMANAGER_H
#define __WIFIMANAGER_H

#include <WiFi.h>
#include <SPIFFS.h>

class WiFiManager {
    String _hostname;
    String _ssid;
    String _password;
    void onDisconnect(WiFiEvent_t event, WiFiEventInfo_t info);
    bool connect2(const char* ssid, const char* password);
    bool connect2(const char* ssid);
    public:
    WiFiManager();
    WiFiManager(String hostname);
    void connect();
    void scan(); 
    void setup3();
    void setup2();
    void setup(fs::FS &fs);
};

class CSV {
    const char* _path;
    public:
    void initSPIFFS();
    CSV(const char* path);
    // TOOD: add a destructor
    void print(fs::FS &fs);
    void addLine(fs::FS &fs, const String& line);
    //String readLine();
    //void deleteLine();
    //void replaceLine();
};

#endif