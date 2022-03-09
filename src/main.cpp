#include "alarmclock.h"

Alarm al;

const char *ssid     = "WirelessNW_2.4";
const char *password = "red66dog";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -18000;
const int   daylightOffset_sec = 3600;

const char* hostname = "ESP32 Alarm Clock";
const bool WiFiEnabled = true;

AiEsp32RotaryEncoder rotaryEncoder = AiEsp32RotaryEncoder(ROTARY_ENCODER_A_PIN, ROTARY_ENCODER_B_PIN, ROTARY_ENCODER_BUTTON_PIN, ROTARY_ENCODER_VCC_PIN, ROTARY_ENCODER_STEPS);

GxEPD2_BW<GxEPD2_290, GxEPD2_290::HEIGHT> display(GxEPD2_290(EINK_CS, EINK_DC, EINK_RST, EINK_BUSY)); // GDEH029A1 128x296

void setupWiFi();
void WiFiConnect();
void drawWiFiIcon();

void IRAM_ATTR readEncoderISR()
{
    rotaryEncoder.readEncoder_ISR();
}

void setup() {
  Serial.begin(115200);
  setupWiFi();
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  setupDFPlayer();
  al.hour = 9;
  al.minute = 6;

  rotaryEncoder.begin();
  rotaryEncoder.setup(readEncoderISR);
  rotaryEncoder.setBoundaries(0, 1000, false); //minValue, maxValue, circleValues true|false (when max go to min and vice versa)
  rotaryEncoder.setAcceleration(250);

  display.init();
  display.setRotation(1);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);  
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


int alreadyDrawn = false;
void loop() 
{
  manageDFPlayer();
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  al.checkAlarm(timeinfo);
  if (rotaryEncoder.encoderChanged())
  {
      Serial.println(rotaryEncoder.readEncoder());
  }
  if (rotaryEncoder.isEncoderButtonClicked())
  {
      Serial.println("button pressed");
  }
  if(!alreadyDrawn) {
    drawWiFiIcon();
    alreadyDrawn = true;
  }
}

// Draws the wifi icon at the top of the screen according to the wifi status
void drawWiFiIcon() {
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    if(!WiFiEnabled) {
      display.drawBitmap(296-24-1, 1, wifi_airplane_mode, 24, 24, GxEPD_BLACK);
    } else {
        if (WiFi.status() == WL_CONNECTED) 
      {
        display.drawBitmap(296-24-1, 1, wifi_connected, 24, 24, GxEPD_BLACK);
      } else {
        display.drawBitmap(296-24-1, 1, wifi_disconnected, 24, 24, GxEPD_BLACK);
      }
    }
  }
  while (display.nextPage());
}