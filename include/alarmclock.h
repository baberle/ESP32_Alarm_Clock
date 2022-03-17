#ifndef __ALARMCLOCK_H
#define __ALARMCLOCK_H

#include <Arduino.h>
#include <time.h>
#include <WiFi.h>
#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include "alarmplayer.h"
#include "alarm.h"
#include "AiEsp32RotaryEncoder.h"
#include "bitmaps.h"
#include <Adafruit_I2CDevice.h> 
//#include "BluetoothSerial.h"

#define ROTARY_ENCODER_A_PIN 32
#define ROTARY_ENCODER_B_PIN 21
#define ROTARY_ENCODER_BUTTON_PIN 25
#define ROTARY_ENCODER_VCC_PIN -1
#define ROTARY_ENCODER_STEPS 4

#define EINK_CS SS
#define EINK_DC 17
#define EINK_RST 16
#define EINK_BUSY 4

void manageLoop();

void setupWiFi();
void WiFiConnect();

void drawPairing();
void drawLoading();
bool checkScreenTimeout();

void mainTimeDisplayLoop();
void drawWiFiIcon();
void drawAlarmIcon();
void displayClockPage(bool partial);

void mainMenuLoop();
void displayTitle();
void displayMainMenu();
void displayMenuSelectionIndicator(int selection);

void alarmsLoop();
void displayAlarms();


#endif