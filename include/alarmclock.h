#ifndef __ALARMCLOCK_H
#define __ALARMCLOCK_H

#include <Arduino.h>
#include <time.h>
#include <WiFi.h>
#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include "alarmplayer.h"
#include "alarm.h"
#include "alarmset.h"
#include "AiEsp32RotaryEncoder.h"
#include "bitmaps.h"
#include <Adafruit_I2CDevice.h> 
#include <string.h>
#include <math.h>
#include "backlight.h"
#include "Preferences.h"
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

#define LED_1 26
#define TOUCH 27

void setupWiFi();
void WiFiConnect();

void testSetup();

bool manageLoop();

void startNightLight();
void turnOffNightLight();
void startLedMomentary();
void manageLED();

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

void timeSettingsLoop();
void displayTimeSettings(bool partial);

void mainSettingsLoop();
void displayMainSettings(bool partial);

void alarmSettingsLoop(Alarm& currentAlarm);
void displayAlarmSettings(bool partial, bool alarmStatus, SnoozeType snoozeStatus);

void alarmSettings2Loop(Alarm& currentAlarm);
void displayAlarmSettings2(bool partial, Alarm& currentAlarm, const int selection, const bool timeChange);
void displayAlarmTime(Alarm& currentAlarm, const bool active, const bool focus);
void displayDayIcon(bool selected, bool active, int wday);

void printLineAlarms(const int x, const int y, const int row);
bool rowActionAlarms(const int row);
void displayAlarmsList();

void printLineTimezone(const int x, const int y, const int row);
bool rowActionTimezone(const int row);
void displayTimezoneList();

void printLineChime(const int x, const int y, const int row);
bool rowActionChime(const int row);
void hoverChime(const int row);
void displayChimeList();

void listLoop(const char* listTitle, const int length, int top, void (*printLine)(int,int,int), bool (*clickAction)(int), void (*onHover)(int));
void displayList(bool partial, int top, const char* title, void (*printLine)(int,int,int));

void mathSnoozeLoop(Alarm& currentAlarm);
void mathSnoozeDisplay(bool partial, const String& equation, const String (&solutions)[3], const int selection);

void displayPopup(const char* message);

#endif