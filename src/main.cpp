#include "alarmclock.h"

Alarm al;

const char *ssid     = "WirelessNW_2.4";
const char *password = "red66dog";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -18000;
const int   daylightOffset_sec = 3600;

const char* hostname = "ESP32 Alarm Clock";
const bool WiFiEnabled = true;

void setupWiFi();
void WiFiConnect();

void setup() {
  Serial.begin(115200);
  setupWiFi();
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  setupDFPlayer();
  al.hour = 9;
  al.minute = 6;
}

void WiFiConnect() {
  long startTime = millis();
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      if(millis() - startTime > 5000) {
        Serial.println(" DISCONNECTED");
        return;
      }
      delay(500);
      Serial.print(".");
  }
  Serial.println(" CONNECTED");
}

void setupWiFi() {
  if(!WiFiEnabled) return;
  WiFi.mode(WIFI_STA);
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.setHostname(hostname);
  //WiFi.onEvent(WiFiStationConnected, SYSTEM_EVENT_STA_CONNECTED);
  //WiFi.onEvent(WiFiStationDisconnected, SYSTEM_EVENT_STA_DISCONNECTED);
  //WiFiScan();
  WiFiConnect();
}



void loop() 
{
  manageDFPlayer();
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  al.checkAlarm(timeinfo);
  delay(100);
}