#include "alarmclock.h"

int prevMinute = 0;
enum Change {none, full, partial};
bool militaryTime = false;

enum Screen {
  clock_scr, 
  main_menu_scr, 
  alarms_scr, 
  alarm_setting_scr,
  alarm_setting2_scr,
  chime_setting_scr,
  timezone_scr, 
  settings_scr
};
Screen screen = clock_scr;
unsigned long timeSinceLastAction = millis();

AlarmSet alarmset;
//Alarm al;
Alarm* currentSelectedAlarm;

const char *ssid     = "WirelessNW_2.4";
const char *password = "red66dog";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -18000;
const int   daylightOffset_sec = 3600;

const char* timeZoneDescription[31] = {
  "Greenwich Mean",
  "European Central",
  "Eastern European",
  "Egypt Standard",
  "Eastern African",
  "Middle East",
  "Near East",
  "Pakistan Lahore",
  "India Standard",
  "Bangladesh Standard",
  "Vietnam Standard",
  "China Taiwan",
  "Japan Standard",
  "Australia Central",
  "Australia Eastern",
  "Solomon Standard",
  "New Zealand Standard",
  "Midway Islands",
  "Hawaii Standard",
  "Alaska Standard",
  "Pacific Standard",
  "Phoeniz Standard",
  "Mountain Standard",
  "Central Standard",
  "Eastern Standard",
  "Indiana Eastern Standard",
  "Puerto Rico",
  "Canada Newfoundland",
  "Argentina Standard",
  "Brazil Eastern",
  "Central African"
};

const int timeZoneOffset[31] = {
  0,
  3600,
  7200,
  7200,
  10800,
  12600,
  14400,
  18000,
  19800,
  21600,
  25200,
  28800,
  32400,
  34200,
  36000,
  39600,
  43200,
  -39600,
  -36000,
  -32400,
  -28800,
  -25200,
  -25200,
  -21600,
  -18000,
  -18000,
  -14400,
  -12600,
  -10800,
  -10800,
  -3600
};

// considerations, such as limiting number of alarms and making number of alarms static, because limited memory
// consider alignment of structs to prevent wasted space
// explanation of the different options and the benefits/downsides of each
//    this could include IDE, microcontroller, microcontroller settings, coding style,
//    other choices for components (slow refresh time)

const char* hostname = "ESP32 Alarm Clock";
const bool WiFiEnabled = true;

AiEsp32RotaryEncoder rotaryEncoder = AiEsp32RotaryEncoder(ROTARY_ENCODER_A_PIN, ROTARY_ENCODER_B_PIN, ROTARY_ENCODER_BUTTON_PIN, ROTARY_ENCODER_VCC_PIN, ROTARY_ENCODER_STEPS);

GxEPD2_BW<GxEPD2_290, GxEPD2_290::HEIGHT> display(GxEPD2_290(EINK_CS, EINK_DC, EINK_RST, EINK_BUSY)); // GDEH029A1 128x296

Channel ch1(0, 255, 8);
Backlight backlight(LED_1, ch1);

int threshold = 40;
bool touchDetected = false;

const char *fileTitles[14] = {
  "Star Wars",
  "Indiana Jones",
  "Tintin",
  "The Cowboys",
  "Dragon's Den",
  "Here Comes the Sun",
  "Classic 1",
  "Classic 2",
  "Morning has Broken",
  "Arcade Fire",
  "Blue Sky",
  "Morning Mood",
  "Call to Cows",
  "Rooster"
};
const int numFileTitles = 14;

unsigned long touchDelay = 0;
//bool alarmIsPlaying = false;
bool hold = false;

void hitSnooze() {
  touchDetected = true;
  Serial.println("Touch detected");
  //startLedMomentary();
  backlight.startMomentary();
}

/*void hitSnooze() {
  if(alarmIsPlaying && !alarmSnoozed) {
    if(!hold) {
      touchDelay = millis();
      hold = true;
    } else {
      const int holdTime = 3000;
      if(millis() - touchDelay > holdTime) {
        hold = false;
        alarm.snooze();
      }
    }
  } else {
    backlight.startMomentary();
  }
}*/

void IRAM_ATTR readEncoderISR()
{
    rotaryEncoder.readEncoder_ISR();
}

void setup() {

  Serial.begin(115200);

  display.init();
  display.setRotation(1);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK); 

  drawLoading();

  setupWiFi();

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  setupDFPlayer();
  stopTrack();

  testSetup();

  rotaryEncoder.begin();
  rotaryEncoder.setup(readEncoderISR);
  rotaryEncoder.setBoundaries(0, 1000, false); //minValue, maxValue, circleValues true|false (when max go to min and vice versa)
  rotaryEncoder.setAcceleration(100); 

  touchAttachInterrupt(TOUCH, hitSnooze, threshold);
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

void testSetup() {

  alarmset.addAlarm();
  alarmset.addAlarm();
  alarmset.addAlarm();
  alarmset.addAlarm();

  alarmset.alarms[0].active = true;
  alarmset.alarms[0].hour = 19;
  alarmset.alarms[0].minute = 40;
  alarmset.alarms[0].day[1] = false;
  alarmset.alarms[0].day[2] = false;

  alarmset.alarms[1].active = true;
  alarmset.alarms[1].hour = 20;
  alarmset.alarms[1].day[0] = false;
  alarmset.alarms[1].day[6] = false;

  alarmset.alarms[2].active = true;
  alarmset.alarms[2].hour = 2;
  alarmset.alarms[2].minute = 30;
  alarmset.alarms[2].day[1] = false;
  alarmset.alarms[2].day[2] = false;
  alarmset.alarms[2].day[3] = false;
  alarmset.alarms[2].day[4] = false;
  alarmset.alarms[2].day[5] = false;

}

void loop() 
{
  
  manageLoop();

  switch(screen) {
    case clock_scr:
      mainTimeDisplayLoop();
      break;
    case main_menu_scr:
      mainMenuLoop();
      break;
    case alarms_scr:
      displayAlarmsList();
      break;
    case alarm_setting_scr:
      alarmSettingsLoop(*currentSelectedAlarm);
      break;
    case alarm_setting2_scr:
      alarmSettings2Loop(*currentSelectedAlarm);
      break;
    case chime_setting_scr:
      displayChimeList();
      break;
    case timezone_scr:
      displayTimezoneList();
      break;
    case settings_scr:
      mainSettingsLoop();
      break;
  } 

}

void manageLoop() {
  manageDFPlayer();
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  //al.checkAlarm(timeinfo);
  alarmset.checkAllAlarms(timeinfo);
  backlight.manageBacklight();
}


void drawLoading() {
  const char conn[] = "Connecting ...";
  int16_t tbx, tby; uint16_t tbw, tbh;
  display.getTextBounds(conn, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t x = ((display.width() - tbw) / 2) - tbx;
  uint16_t y = (display.height() - tbh);
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.drawBitmap(15, 20, clock_logo, 259, 54, GxEPD_BLACK);
    display.setCursor(x,y);
    display.print(conn);
  }
  while (display.nextPage());
}

bool checkScreenTimeout() {
  const int timeoutInterval = 2*60*1000;
  if(millis() - timeSinceLastAction > timeoutInterval) {
    if(screen != clock_scr) {
      screen = clock_scr;
      return true;
    }
    timeSinceLastAction = millis();
  }
  return false;
}



/* =========================== DISPLAY CLOCK =========================== */



// Checks if the time has changed and returns the refresh type for the clock display
Change minuteHasChanged() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return none;
  }
  int minute = timeinfo.tm_min;
  int minB = minute%10;
  if(minB != prevMinute) {
    prevMinute = minB;
    if(minB % 10 == 0) {
      return full;
    } else {
      return partial;
    }
  } else {
    return none;
  }
}

// Handles everything for the main time screen
void mainTimeDisplayLoop() {
  if(Serial) Serial.println("Displaying main clock");
  displayClockPage(false);
  while(true) {
    manageLoop();
    // Change display if time has changed
    Change ch = minuteHasChanged();
    if(ch == full) {
      displayClockPage(false);
    } else 
    if(ch == partial) {
      displayClockPage(true);
    }
    // conditions to change state
    if(rotaryEncoder.isEncoderButtonClicked()) {
      screen = main_menu_scr;
      break;
    }
  }
}

// Draws the wifi icon at the top of the screen according to the wifi status
void drawWiFiIcon() {
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

// Draws the alarm icon at the top of the screen
void drawAlarmIcon() {
  for(int i = 0; i < alarmset.numAlarms; i++) {
    if(alarmset.alarms[i].active) {
      display.drawBitmap(296-24-24-2, 1, alarm_icon, 24, 24, GxEPD_BLACK); 
      break;
    }
  }
}

// Gets the time, formats it, and displays it on the screen
void displayClockPage(bool partial) {
  
  int height = 30;
  int left = 10;

  struct tm timeinfo;
  char dateStringBuff[30];
  bool morning = false;
  
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }

  int hour;
  if(!militaryTime){
    hour = timeinfo.tm_hour % 12;
  } else {
    hour = timeinfo.tm_hour;
  }
  if(timeinfo.tm_hour < 12) morning = true;
  if(hour == 0) hour = 12;
  int minute = timeinfo.tm_min;

  int hourA = hour/10;
  int hourB = hour%10;
  int minA = minute/10;
  int minB = minute%10;

  strftime(dateStringBuff, sizeof(dateStringBuff), "%A, %b %e", &timeinfo);

  if(partial) {
    display.setPartialWindow(0, 0, display.width(), display.height());
  } else {
    display.setFullWindow();
  }
  
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    
    display.setCursor(1, 12);
    display.print(dateStringBuff);
    
    if(hourA == 0) {
      display.drawBitmap(left+60-28, height, digit_allArray[hourB], 56, 85, GxEPD_BLACK);
      display.fillCircle(left+60*2+10-28, 55, 5, GxEPD_BLACK);
      display.fillCircle(left+60*2+10-28, 90, 5, GxEPD_BLACK);
      display.drawBitmap(left+20+60*2-28, height, digit_allArray[minA], 56, 85, GxEPD_BLACK);
      display.drawBitmap(left+20+60*3-28, height, digit_allArray[minB], 56, 85, GxEPD_BLACK);
      display.setCursor(265-28, 111);
    } else {
      display.drawBitmap(left, height, digit_allArray[hourA], 56, 85, GxEPD_BLACK);
      display.drawBitmap(left+60, height, digit_allArray[hourB], 56, 85, GxEPD_BLACK);
      display.fillCircle(left+60*2+10, 55, 5, GxEPD_BLACK);
      display.fillCircle(left+60*2+10, 90, 5, GxEPD_BLACK);
      display.drawBitmap(left+20+60*2, height, digit_allArray[minA], 56, 85, GxEPD_BLACK);
      display.drawBitmap(left+20+60*3, height, digit_allArray[minB], 56, 85, GxEPD_BLACK);
      display.setCursor(265, 111);
    }

    if(!militaryTime) {
      if(morning) {
        display.print("AM");
      }
      else {
        display.print("PM");
      }
    }

    drawWiFiIcon();
    drawAlarmIcon();
    
  }
  while (display.nextPage());
}




/* =========================== DISPLAY MAIN MENU =========================== */




void mainMenuLoop() {

  if(Serial) Serial.println("Displaying main menu");
  
  timeSinceLastAction = millis();
  rotaryEncoder.setBoundaries(0, 3, false);
  rotaryEncoder.setEncoderValue(0);
  displayMainMenu();

  while(true) {

    if(rotaryEncoder.encoderChanged()) {
      timeSinceLastAction = millis();
      displayMenuSelectionIndicator(rotaryEncoder.readEncoder());
    }
    if(checkScreenTimeout()) return;
    manageLoop();
    
    if(rotaryEncoder.isEncoderButtonClicked()) {
      switch(rotaryEncoder.readEncoder()) {
        case 0:
          screen = alarms_scr;
          return;
        case 1:
          break;
        case 2:
          screen = settings_scr;
          return;
        case 3:
          screen = clock_scr;
          return;
      }
    }
  }
}


void displayTitle(const char* title) {
  int16_t tbx, tby;
  uint16_t tbw, tbh;
  display.getTextBounds(title, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t x = ((display.width() - tbw) / 2) - tbx;
  display.fillRect(0, 0, display.width(), 24, GxEPD_BLACK);
  display.setCursor(x, 16);
  display.setTextColor(GxEPD_WHITE);
  display.print(title);
  display.setTextColor(GxEPD_BLACK);
}


void displayMainMenu() {

  display.setFullWindow();
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);

    displayTitle("Main Menu");
    
    display.setCursor(30, 22*2);
    display.print("Alarms");
    
    display.setCursor(30, 22*3);
    display.print("Set Time");
    
    display.setCursor(30, 22*4);
    display.print("Settings");
    
    display.setCursor(30, 22*5);
    display.print("Exit");
  }
  while (display.nextPage());

  displayMenuSelectionIndicator(rotaryEncoder.readEncoder());
}


void displayMenuSelectionIndicator(int selection) {
  display.setPartialWindow(0, 24, 29, display.height());
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.fillCircle(15, 16+22*(selection+1), 3, GxEPD_BLACK);
  }
  while (display.nextPage());
}



/* =========================== DISPLAY ALARMS =========================== */

void displayAlarmLine(Alarm& alarm, const int x, const int y) {
  // TODO: not everything in terms of x
  char timeString[10];
  alarm.toString(militaryTime, timeString);
  display.setCursor(x, y);
  display.print(timeString);

  display.drawLine(130, y, 130, y-22, GxEPD_BLACK);

  char dayString[10];
  alarm.toDayString(dayString);
  int16_t tbx, tby; uint16_t tbw, tbh;
  display.getTextBounds(dayString, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t x1 = ((120 - tbw) / 2) - tbx;
  display.setCursor(130+x1, y);
  display.print(dayString);

  display.drawLine(250, y, 250, y-22, GxEPD_BLACK);

  display.setCursor(260, y);
  display.print(alarm.active ? "ON" : "OFF");
}

void printLineAlarms(const int x, const int y, const int row) {
  if(row < alarmset.numSetAlarms) {
      displayAlarmLine(alarmset.alarms[row], x, y);
    } else {
      if(row == alarmset.numSetAlarms) {
        display.drawBitmap(x, y-14, plus_outline, 18, 18, GxEPD_BLACK);
        display.setCursor(x+18+4, y);
        display.print("New Alarm");
      } else 
      if(row == alarmset.numSetAlarms+1) {
        display.setCursor(x, y);
        display.print("Exit");
      }
    }
}

bool rowActionAlarms(const int row) {
  if(row < alarmset.numSetAlarms) {
    // go to alarm setsing screen for alarm # row
    Serial.println("Go into alarm");
    currentSelectedAlarm = &alarmset.alarms[row];
    screen = alarm_setting_scr;
    return true;
  } else 
  if(row == alarmset.numSetAlarms) {
    // Add new alarm logic
    // TODO: go to alarm setting screen if room for another alarm
    Serial.println("Making new alarm");
    alarmset.addAlarm();
  } else 
  if(row == alarmset.numSetAlarms+1) {
    screen = main_menu_scr;
    return true;
  }
  return false;
}

void displayAlarmsList() {
    Serial.println("Entering Alarms List Screen");
    Serial.print("Num alarms is ");Serial.println(alarmset.numSetAlarms);
    listLoop("Alarms", alarmset.numSetAlarms+1, 0, &printLineAlarms, &rowActionAlarms, NULL);
}

/* =========================== CHOOSE TIMEZONES =========================== */

void printLineTimezone(const int x, const int y, const int row) {
    if(row > 30) return;
    display.setCursor(x, y);
    display.print(timeZoneDescription[row]);
}

bool rowActionTimezone(const int row) {
  // TODO: change timezone offset (with correct offset for daylight savings time)
  if(row < 0 || row > 30) return false;
  Serial.print("New offset is: ");
  Serial.println(timeZoneOffset[row]);
  configTime(timeZoneOffset[row], daylightOffset_sec, ntpServer);
  screen = settings_scr;
  return true;
}

void displayTimezoneList() {
    Serial.println("Entering Timezone List Screen");
    listLoop("Timezones", 30, 24, &printLineTimezone, &rowActionTimezone, NULL);
    // TODO: Is this the correct number of timezones?
}

/* =========================== MAIN SETTIGNS SCREEN =========================== */


void mainSettingsLoop() {

  if(Serial) Serial.println("Displaying settings menu");

  rotaryEncoder.setBoundaries(0, 3, false);
  rotaryEncoder.setEncoderValue(0);
  displayMainSettings(false);
  timeSinceLastAction = millis();

  while(true) {

    if(checkScreenTimeout()) return;
    manageLoop();

    if(rotaryEncoder.encoderChanged()) {
      timeSinceLastAction = millis();
      displayMenuSelectionIndicator(rotaryEncoder.readEncoder());
    }
    
    if(rotaryEncoder.isEncoderButtonClicked()) {
      switch(rotaryEncoder.readEncoder()) {
        case 0:
          militaryTime = !militaryTime;
          displayMainSettings(true);
          break;
        case 1:
          screen = timezone_scr;
          return;
        case 2:
          if(backlight.getState()) backlight.turnOff();
          else backlight.turnOff();
          displayMainSettings(true);
          break;
        case 3:
          screen = main_menu_scr;
          return;
      }
    }
  }
}


void displayMainSettings(bool partial) {

  if(partial) display.setPartialWindow(0, 0, display.width(), display.height());
  else display.setFullWindow();
  
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    
    displayTitle("Settings");
    
    display.setCursor(30, 22*2);
    if(militaryTime) {
      display.print("24 Hour Time: ON");
    } else {
      display.print("24 Hour Time: OFF");
    }    
    
    display.setCursor(30, 22*3);
    display.print("Change Timezone");
    
    display.setCursor(30, 22*4);
    if(backlight.getState()) {
      display.print("Night Light: ON");
    } else {
      display.print("Night Light: OFF");
    } 
    
    display.setCursor(30, 22*5);
    display.print("Exit");
  }
  while (display.nextPage());

  displayMenuSelectionIndicator(rotaryEncoder.readEncoder());
}

/* =========================== ALARM SETTIGNS 1 =========================== */

void alarmSettingsLoop(Alarm& currentAlarm) {

  if(Serial) Serial.println("Displaying alarm settings menu");

  rotaryEncoder.setBoundaries(0, 3, false);
  rotaryEncoder.setEncoderValue(0);
  displayAlarmSettings(false, currentAlarm.active, currentAlarm.snooze);
  timeSinceLastAction = millis();

  while(true) {

    if(checkScreenTimeout()) return;
    manageLoop();

    if(rotaryEncoder.encoderChanged()) {
      timeSinceLastAction = millis();
      displayMenuSelectionIndicator(rotaryEncoder.readEncoder());
    }
    
    if(rotaryEncoder.isEncoderButtonClicked()) {
      switch(rotaryEncoder.readEncoder()) {
        case 0:
          currentAlarm.active = !currentAlarm.active;
          displayAlarmSettings(true, currentAlarm.active, currentAlarm.snooze);
          break;
        case 1:
          currentAlarm.snooze = !currentAlarm.snooze;
          displayAlarmSettings(true, currentAlarm.active, currentAlarm.snooze);
          break;
        case 2:
          screen = chime_setting_scr;
          return;
        case 3:
          screen = alarm_setting2_scr;
          return;
      }
    }
  }
}

void displayAlarmSettings(bool partial, bool alarmStatus, bool snoozeStatus) {

  if(partial) display.setPartialWindow(0, 0, display.width(), display.height());
  else display.setFullWindow();
  
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    
    displayTitle("Alarm Settings");
    
    display.setCursor(30, 22*2);
    if(alarmStatus) {
      display.print("Alarm: ON");
    } else {
      display.print("Alarm: OFF");
    } 
    
    display.setCursor(30, 22*3);
    if(snoozeStatus) {
      display.print("Snooze: ON");
    } else {
      display.print("Snooze: OFF");
    }  
    
    display.setCursor(30, 22*4);
    display.print("Set Chime");
    
    display.setCursor(30, 22*5);
    display.print("Next");
  }
  while (display.nextPage());

  displayMenuSelectionIndicator(rotaryEncoder.readEncoder());
}


/* =========================== ALARM SETTIGNS 2 =========================== */

void alarmSettings2Loop(Alarm& currentAlarm) {

  if(Serial) Serial.println("Displaying alarm settings 2 menu");

  rotaryEncoder.setBoundaries(0, 8, false);
  rotaryEncoder.setEncoderValue(0);
  displayAlarmSettings2(false, currentAlarm, 0, false);
  timeSinceLastAction = millis();

  int selection = 0;
  bool changeTime = false;

  while(true) {

    if(checkScreenTimeout()) return;
    manageLoop();

    if(rotaryEncoder.encoderChanged()) {
      timeSinceLastAction = millis();
      if(changeTime) {
        // TODO: bug on top boundary
        currentAlarm.setTime(rotaryEncoder.readEncoder()*5);
        displayAlarmSettings2(true, currentAlarm, 0, true);
      } else {
        selection = rotaryEncoder.readEncoder();
        displayAlarmSettings2(true, currentAlarm, selection, false);
      }
    }
    
    if(rotaryEncoder.isEncoderButtonClicked()) {
      int val = rotaryEncoder.readEncoder();
      switch((changeTime ? 0 : val)) {
        case 0:
          if(!changeTime) {
            rotaryEncoder.setBoundaries(0, 287, true);
            rotaryEncoder.setEncoderValue(12*currentAlarm.hour + currentAlarm.minute/5);
            changeTime = true;
          } else {
            rotaryEncoder.setBoundaries(0, 8, false);
            rotaryEncoder.setEncoderValue(0);
            selection = 0;
            changeTime = false;
          }
          displayAlarmSettings2(true, currentAlarm, selection, changeTime);
          break;
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
          currentAlarm.day[val-1] = !currentAlarm.day[val-1];
          displayAlarmSettings2(true, currentAlarm, selection, false);
          break;
        case 8:
          screen = main_menu_scr;
          return;
        default: break;
      }
    }
  }
}


void displayAlarmSettings2(bool partial, Alarm& currentAlarm, const int selection, const bool timeChange) {

  if(partial) display.setPartialWindow(0, 0, display.width(), display.height());
  else display.setFullWindow();
  
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    
    displayTitle("Alarm Settings");
    
    if(selection == 0) displayAlarmTime(currentAlarm, true, timeChange);
    else displayAlarmTime(currentAlarm, false, timeChange);

    for(int wday = 0; wday < 7; wday++) {
      if(selection == wday+1) displayDayIcon(true, currentAlarm.day[wday], wday);
      else displayDayIcon(false, currentAlarm.day[wday], wday);
    }
    display.setCursor(130,120);
    display.setTextColor(GxEPD_BLACK);
    display.print("Exit");
    display.setCursor(120,120);
    if(selection == 8) display.fillCircle(120, 115, 3, GxEPD_BLACK);
  }
  while (display.nextPage());
}

// TODO account for 12-hour time where first digit not displayed
void displayAlarmTime(Alarm& currentAlarm, const bool active, const bool focus) {

  display.setCursor(115, 55);
  display.setTextColor(GxEPD_BLACK);

  char alarmTime[10];
  currentAlarm.toString(militaryTime, alarmTime);
  display.print(alarmTime);

  int width = (militaryTime ? 60 : 90);
  if(active) {
    display.fillRect(115, 60/*y*/, width, 2/*height*/, GxEPD_BLACK);
  }
  if(focus) {
    display.fillRect(115, 64/*y*/, width, 2/*height*/, GxEPD_BLACK);
  }
}

void displayDayIcon(bool selected, bool active, int wday) {
  const char weekSymbol[7] = {'S','M','T','W','T','F','S'};
  if(selected) {
    display.drawCircle(37*(wday+1), 90, 14, GxEPD_BLACK);
  }
  if(active) {
    display.setTextColor(GxEPD_WHITE);
    display.fillCircle(37*(wday+1), 90, 12, GxEPD_BLACK);
  } else {
    display.setTextColor(GxEPD_BLACK);
    display.fillCircle(37*(wday+1), 90, 12, GxEPD_WHITE);
  }
  display.setCursor(37*(wday+1)-5, 90+5);
  display.print(weekSymbol[wday]);
}


/* =========================== CHIME SELECTION =========================== */

void printLineChime(const int x, const int y, const int row) {
    if(row < 0 || row > numFileTitles-1) return;
    display.setCursor(x, y);
    display.print(fileTitles[row]);
}

bool rowActionChime(const int row) {
    if(row < 0 || row > numFileTitles-1) return false;
      Serial.print("New chime is: ");
      Serial.println(fileTitles[row]);
      currentSelectedAlarm->ap.track = row;
      screen = alarm_setting_scr;
      stopTrack();
      return true;
}

void hoverChime(const int row) {
    playTrack(row+1);
}

void displayChimeList() {
    Serial.println("Entering Chime List Screen");
    listLoop("Chime", numFileTitles, 0, &printLineChime, &rowActionChime, &hoverChime);
}

/* =========================== LIST DISPLAY =========================== */

void listLoop(const char* title, const int length, int top, void (*printLine)(int,int,int), bool (*clickAction)(int), void (*onHover)(int)) {

  int prevEncoderPostition = 0;
  //const int numLines = 4;
  
  timeSinceLastAction = millis();
  rotaryEncoder.setBoundaries(0, length, false);
  rotaryEncoder.setEncoderValue(top);
  displayList(false, top, title, printLine);
  displayMenuSelectionIndicator(0);

  while(true) {

    if(checkScreenTimeout()) return;
    manageLoop();

    if(rotaryEncoder.encoderChanged()) {
        timeSinceLastAction = millis();
        int val = rotaryEncoder.readEncoder();
        if(onHover != NULL) onHover(val);
        if(val == top && top != 0 && val < prevEncoderPostition) {
            prevEncoderPostition = val;
            // TODO: should increade by possibly more then once because the library (but make sure it doesn't overstep)
            top--;
            displayList(true, top, title, printLine);
            displayMenuSelectionIndicator(0);
        } else
        if(val == top+4 && top != length && val > prevEncoderPostition) {
            prevEncoderPostition = val;
            top++;
            displayList(true, top, title, printLine);
            displayMenuSelectionIndicator(3);
        } else {
            displayMenuSelectionIndicator(val - top);
        }
    }
    
    if(rotaryEncoder.isEncoderButtonClicked()) {
      if(clickAction(rotaryEncoder.readEncoder())) return;
    }

  }
}

void displayList(bool partial, int top, const char* title, void (*printLine)(int,int,int)) {

  if(partial) display.setPartialWindow(0, 0, display.width(), display.height());
  else display.setFullWindow();

  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);

    displayTitle(title);

    const int lineHeight = 22;
    const int horizontalOffset = 30;
    const int numLines = 4;

    int row = 0;
    for(int i = top; i < top+numLines; i++) {
      int verticalOffset = lineHeight*(row+2);
      printLine(horizontalOffset, verticalOffset, i);
      row++;
    }

  }
  while (display.nextPage());
}