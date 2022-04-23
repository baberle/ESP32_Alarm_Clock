#include "alarmclock.h"

Preferences pref;

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
  settings_scr,
  snooze_math_scr,
  time_settings_scr
};
Screen screen = clock_scr;
//Screen screen = alarms_scr;
unsigned long timeSinceLastAction = millis();

AlarmGroup alarmgroup;
Alarm* currentSelectedAlarm;
int currentSelectedAlarmIdx;
Alarm* alarmGoingOff;

/*const char *ssid     = "WirelessNW_2.4";
const char *password = "red66dog";*/
/*const char *ssid = "bphone";
const char *password = "espnetwork";*/

const char* ntpServer = "pool.ntp.org";
long  gmtOffset_sec = -18000;
int   daylightOffset_sec = 3600;

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
  "Phoenix Standard",
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
// https://publib.boulder.ibm.com/tividd/td/TWS/SC32-1274-02/en_US/HTML/SRF_mst273.htm

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

const int threshold = 40;

const char *fileTitles[14] = {
  "Star Wars",
  "Indiana Jones",
  "Tintin",
  "The Cowboys",
  "Dragon's Den",
  "Here Comes the Sun",
  "Classic 1",
  "Classic 2",
  "Arcade Fire",
  "Blue Sky",
  "Morning has Broken",
  "Morning Mood",
  "Call to Cows",
  "Rooster"
};
const int numFileTitles = 14;

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

  /*initSPIFFS();

  ssid = readFile(SPIFFS, ssidPath);
  pass = readFile(SPIFFS, passPath);
  ip = readFile(SPIFFS, ipPath);
  gateway = readFile (SPIFFS, gatewayPath);
  Serial.println(ssid);
  Serial.println(pass);
  Serial.println(ip);
  Serial.println(gateway);

  deliverWebpage();*/


  //wifi_manager.setup2();

  //wifi_manager.setup();

  setupWiFi();
  if(inApMode()) {
    displayApMode(getIpAddr().c_str());
  }

  // TODO: should maybe also read this way and not store in global var?
  pref.begin("sett",false);
  militaryTime = pref.getBool("mil", false);
  gmtOffset_sec = pref.getInt("gmt_off", -18000);
  daylightOffset_sec = (pref.getBool("dst",true) ? 3600 : 0);
  if(pref.getBool("light", false)) backlight.turnOn();

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  //setupWiFi();

  setupDFPlayer();
  stopTrack();

  //testSetup();
  /*alarmgroup.add();
  alarmgroup.at(0)->active = true;
  alarmgroup.at(0)->hour = 13;
  alarmgroup.at(0)->minute = 23;*/

  rotaryEncoder.begin();
  rotaryEncoder.setup(readEncoderISR);
  rotaryEncoder.setBoundaries(0, 1000, false); //minValue, maxValue, circleValues true|false (when max go to min and vice versa)
  rotaryEncoder.setAcceleration(100); 

  //touchAttachInterrupt(TOUCH, hitSnooze, threshold);

  randomSeed(analogRead(39));

  alarmgroup.printFile(SPIFFS);
  alarmgroup.readFile();
}

void testSetup() {

  alarmgroup.add();
  alarmgroup.add();
  alarmgroup.add();
  alarmgroup.add();

  alarmgroup.at(0)->active = true;
  alarmgroup.at(0)->hour = 19;
  alarmgroup.at(0)->minute = 40;
  alarmgroup.at(0)->day[1] = false;
  alarmgroup.at(0)->day[2] = false;

  alarmgroup.at(1)->active = true;
  alarmgroup.at(1)->hour = 11;
  alarmgroup.at(1)->minute = 46;
  alarmgroup.at(1)->day[0] = false;
  alarmgroup.at(1)->day[6] = false;
  alarmgroup.at(1)->snooze = math;

  alarmgroup.at(2)->active = true;
  alarmgroup.at(2)->hour = 2;
  alarmgroup.at(2)->minute = 30;
  alarmgroup.at(2)->day[1] = false;
  alarmgroup.at(2)->day[2] = false;
  alarmgroup.at(2)->day[3] = false;
  alarmgroup.at(2)->day[4] = false;
  alarmgroup.at(2)->day[5] = false;

}

void loop() 
{
  
  //manageLoop();

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
    case snooze_math_scr:
      mathSnoozeLoop(*alarmGoingOff);
      break;
    case time_settings_scr:
      timeSettingsLoop();
      break;
  } 

}

unsigned long touchDelay = 0;
//bool alarmIsPlaying = false;
bool hold = false;

/*void hitSnooze() {
  Serial.println("Touch detected");
  //if(alarmGoingOff != NULL && )
  /*if(millis() - touchDelay > 1000) {
    alarmgroup.hitSnooze();
  }*/
/*
  alarmgroup.hitOff();
  backlight.startMomentary();
}*/

bool backlightOnBefore = false;
bool alarmOnBefore = false;
bool alreadyTurnedOff = true;
bool alarmSnoozed = false;

unsigned long timeSinceStart;
bool pressed = false;

/*if(touchRead(TOUCH) > threshold) {
  backlight.startMomentary();
  if(pressed && millis() - timeSinceStart > 3000) {
    alarmgroup.hitOff();
    timeSinceStart = millis();
  } else {
    pressed = true;
    timeSinceStart = millis();
  }
} else {
  if(pressed) {
    pressed = false;
    alarmgroup.hitSnooze();
  }
}*/


// TODO: not everything in here has to be checked every loop; maybe split it up a bit
bool manageLoop() {

  bool rVal = false;

  manageDFPlayer();

  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return false;
  }

  // Goes in never ending loop
  Alarm* returnAlarm = alarmgroup.checkAll(timeinfo);
  /*if(returnAlarm != nullptr && !alarmOnBefore) {
    Serial.println("First time alarm going off");
    alreadyTurnedOff = false;
    alarmGoingOff = returnAlarm;
    alarmOnBefore = true;
    backlightOnBefore = backlight.getState();
    backlight.turnOn();
    if(returnAlarm->snooze == math) {
      // TODO: When alarm goes off this should disappear
      screen = snooze_math_scr;
      rVal = true;
    }
  } else if(!alreadyTurnedOff && (returnAlarm == nullptr || returnAlarm->ap.snoozed)) {
    Serial.println("Last time alarm going off");
    if(!backlightOnBefore) backlight.turnOff();
    alreadyTurnedOff = true;
    alarmOnBefore = false;
  }*/

  /*if(returnAlarm != nullptr && returnAlarm->ap.snoozed) {
    if(backlight.getState()) backlight.turnOff();
  } else if(returnAlarm != nullptr && returnAlarm->ap.alarmEnabled && !alarmOnBefore) {
    alarmOnBefore = true;
    if(!backlight.getState()) backlight.turnOn();
  } else if(returnAlarm == nullptr) {
    if(!backlightOnBefore) backlight.turnOff();
    alarmOnBefore = false;
  }*/

  const int holdToTurnOff = 5000;
  if(touchRead(TOUCH) < threshold) {
    backlight.startMomentary();
    if(pressed && millis() - timeSinceStart > holdToTurnOff) {
      alarmgroup.hitOff();
      timeSinceStart = millis();
    } else if(!pressed) {
      pressed = true;
      timeSinceStart = millis();
    }
  } else {
    if(pressed) {
      pressed = false;
      alarmgroup.hitSnooze();
    }
  }

  backlight.manageBacklight();

  if(checkScreenTimeout()) rVal = true;

  // ESP32 will be about 52 seconds off after 30 days
  // Display error after this time
  unsigned long innacurateAfter = 2592000000;
  if(timeDisconnected() != 0) {
    unsigned long duration;
    // handle millis rollover
    if(millis() > timeDisconnected()) {
      duration = millis();
      duration += timeDisconnected() - (unsigned long)0 - (unsigned long)1;
    } else duration = millis() - timeDisconnected();
    if(duration >= innacurateAfter) {
      displayError("Too long disconnect from wifi");
    }
  }

  return rVal;
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
    display.drawBitmap(20, 15, clock_logo, 249, 71, GxEPD_BLACK);
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
    if(manageLoop()) return;
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
  for(int i = 0; i < alarmgroup.size(); i++) {
    if(alarmgroup.at(i)->active) {
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
    //if(checkScreenTimeout()) return;
    if(manageLoop()) return;
    
    if(rotaryEncoder.isEncoderButtonClicked()) {
      switch(rotaryEncoder.readEncoder()) {
        case 0:
          screen = alarms_scr;
          return;
        case 1:
          screen = time_settings_scr;
          return;
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
  if(row < alarmgroup.size()) {
      displayAlarmLine(*alarmgroup.at(row), x, y);
    } else {
      if(row == alarmgroup.size()) {
        display.drawBitmap(x, y-14, plus_outline, 18, 18, GxEPD_BLACK);
        display.setCursor(x+18+4, y);
        display.print("New Alarm");
      } else 
      if(row == alarmgroup.size()+1) {
        display.setCursor(x, y);
        display.print("Exit");
      }
    }
}

bool rowActionAlarms(const int row) {
  if(row < alarmgroup.size()) {
    // go to alarm setsing screen for alarm # row
    Serial.println("Go into alarm");
    currentSelectedAlarm = alarmgroup.at(row);
    currentSelectedAlarmIdx = row;
    screen = alarm_setting_scr;
    return true;
  } else 
  if(row == alarmgroup.size()) {
    // Add new alarm logic
    // TODO: go to alarm setting screen if room for another alarm
    Serial.println("Making new alarm");
    Alarm* newAlarm = alarmgroup.add();
    if(newAlarm != nullptr) {
      screen = alarm_setting_scr;
      currentSelectedAlarm = newAlarm;
      currentSelectedAlarmIdx = alarmgroup.size() - 1;
      return true;
    } else {
      displayPopup("Max Alarms Created");
    }
  } else 
  if(row == alarmgroup.size()+1) {
    screen = main_menu_scr;
    return true;
  }
  return false;
}

void displayAlarmsList() {
    Serial.println("Entering Alarms List Screen");
    Serial.print("Num alarms is ");Serial.println(alarmgroup.size());
    listLoop("Alarms", alarmgroup.size()+1, 0, &printLineAlarms, &rowActionAlarms, NULL);
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
  pref.putInt("gmt_off", timeZoneOffset[row]);
  configTime(timeZoneOffset[row], daylightOffset_sec, ntpServer);
  screen = settings_scr;
  return true;
}

void displayTimezoneList() {
    Serial.println("Entering Timezone List Screen");
    int idx = 0;
    for(; timeZoneOffset[idx] != gmtOffset_sec && idx < 31; idx++);
    listLoop("Timezones", 30, idx, &printLineTimezone, &rowActionTimezone, NULL);
}

/* =========================== MAIN SETTIGNS SCREEN =========================== */


void mainSettingsLoop() {

  if(Serial) Serial.println("Displaying settings menu");

  rotaryEncoder.setBoundaries(0, 2, false);
  rotaryEncoder.setEncoderValue(0);
  displayMainSettings(false);
  timeSinceLastAction = millis();

  while(true) {

    if(manageLoop()) return;

    if(rotaryEncoder.encoderChanged()) {
      timeSinceLastAction = millis();
      displayMenuSelectionIndicator(rotaryEncoder.readEncoder());
    }
    
    if(rotaryEncoder.isEncoderButtonClicked()) {
      switch(rotaryEncoder.readEncoder()) {
        case 0: 
          break;
        case 1:
          break;
        case 1:
          if(backlight.getState()) backlight.turnOff();
          else backlight.turnOn();
          pref.putBool("light", backlight.getState());
          displayMainSettings(true);
          break;
        case 2:
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
    if(WiFi.status() == WL_CONNECTED) display.print("Wi-Fi: Connected");
    else if(inApMode()) display.print("Wi-Fi: Access Point");
    else display.print("Wi-Fi: Disconnected");

    display.setCursor(30, 22*2);
    if(WiFi.status() == WL_CONNECTED) display.print("Wi-Fi: Connected");
    else if(inApMode()) display.print("Wi-Fi: Access Point");
    else display.print("Wi-Fi: Disconnected");
    
    display.setCursor(30, 22*4);
    if(inApMode()) {
      String str = "AP Mode: " + getIpAddr();
      display.print(str);
    } else {
      display.print("AP Mode: --");
    } 
    
    display.setCursor(30, 22*5);
    display.print("Exit");
  }
  while (display.nextPage());

  displayMenuSelectionIndicator(rotaryEncoder.readEncoder());
}

/* =========================== TIME SETTINGS SCREEN =========================== */


void timeSettingsLoop() {

  if(Serial) Serial.println("Displaying time settings menu");

  rotaryEncoder.setBoundaries(0, 3, false);
  rotaryEncoder.setEncoderValue(0);
  displayTimeSettings(false);
  timeSinceLastAction = millis();

  while(true) {

    if(manageLoop()) return;

    if(rotaryEncoder.encoderChanged()) {
      timeSinceLastAction = millis();
      displayMenuSelectionIndicator(rotaryEncoder.readEncoder());
    }
    
    if(rotaryEncoder.isEncoderButtonClicked()) {
      switch(rotaryEncoder.readEncoder()) {
        case 0:
          militaryTime = !militaryTime;
          pref.putBool("mil",militaryTime);
          displayTimeSettings(true);
          break;
        case 1:
          screen = timezone_scr;
          return;
        case 2:
          if(daylightOffset_sec == 3600) {
            daylightOffset_sec = 0;
            pref.putBool("dst",false);
          } else {
            daylightOffset_sec = 3600;
            pref.putBool("dst",true);
          }  
          configTime(pref.getInt("gmt_off",-18000), daylightOffset_sec, ntpServer);
          displayTimeSettings(true);
          break;
        case 3:
          screen = main_menu_scr;
          return;
      }
    }
  }
}


void displayTimeSettings(bool partial) {

  if(partial) display.setPartialWindow(0, 0, display.width(), display.height());
  else display.setFullWindow();
  
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    
    displayTitle("Time Settings");
    
    display.setCursor(30, 22*2);
    if(militaryTime) {
      display.print("24 Hour Time: ON");
    } else {
      display.print("24 Hour Time: OFF");
    }    
    
    display.setCursor(30, 22*3);
    display.print("Change Timezone");
    
    display.setCursor(30, 22*4);
    if(daylightOffset_sec == 3600) {
      display.print("Daylight Savings: ON");
    } else {
      display.print("Daylight Savings: OFF");
    } 
    
    display.setCursor(30, 22*5);
    display.print("Exit");
  }
  while (display.nextPage());

  displayMenuSelectionIndicator(rotaryEncoder.readEncoder());
}

/* =========================== ALARM SETTIGNS 1 =========================== */

void deleteDot(bool on) {
  display.setPartialWindow(170, 8+22*4, 30, 30); // TODO: probably need to play around with the dimension
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    if(on) display.fillCircle(185, 16+22*4, 3, GxEPD_BLACK);
  }
  while (display.nextPage());
}

void alarmSettingsLoop(Alarm& currentAlarm) {

  if(Serial) Serial.println("Displaying alarm settings menu");

  currentAlarm.prevent = true;

  rotaryEncoder.setBoundaries(0, 4, false);
  rotaryEncoder.setEncoderValue(0);
  displayAlarmSettings(false, currentAlarm.active, currentAlarm.snooze);
  timeSinceLastAction = millis();

  int prevEncoderPosition = 0;

  while(true) {

    //if(checkScreenTimeout()) return;
    if(manageLoop()) return;

    if(rotaryEncoder.encoderChanged()) {
      timeSinceLastAction = millis();
      if(rotaryEncoder.readEncoder() == 4) {
        displayMenuSelectionIndicator(10);
        deleteDot(true);
      } else if(prevEncoderPosition == 4) {
        deleteDot(false);
        displayMenuSelectionIndicator(rotaryEncoder.readEncoder());
      } else {
        displayMenuSelectionIndicator(rotaryEncoder.readEncoder());
      }
      prevEncoderPosition = rotaryEncoder.readEncoder();
    }
    
    if(rotaryEncoder.isEncoderButtonClicked()) {
      switch(rotaryEncoder.readEncoder()) {
        case 0:
          currentAlarm.active = !currentAlarm.active;
          displayAlarmSettings(true, currentAlarm.active, currentAlarm.snooze);
          break;
        case 1:
          if(currentAlarm.snooze == on) currentAlarm.snooze = math;
          else if(currentAlarm.snooze == math) currentAlarm.snooze = off;
          else currentAlarm.snooze = on;
          displayAlarmSettings(true, currentAlarm.active, currentAlarm.snooze);
          break;
        case 2:
          screen = chime_setting_scr;
          return;
        case 3:
          screen = alarm_setting2_scr;
          return;
        case 4:
          screen = main_menu_scr;
          alarmgroup.remove(currentSelectedAlarmIdx); // HACK: mixing global and local
          alarmgroup.writeToFile();
          return;
      }
    }
  }
}

void displayAlarmSettings(bool partial, bool alarmStatus, SnoozeType snoozeStatus) {

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
    if(snoozeStatus == on) {
      display.print("Snooze: ON");
    } else if(snoozeStatus == off) {
      display.print("Snooze: OFF");
    } else {
      display.print("Snooze: MATH");
    }
    
    display.setCursor(30, 22*4);
    display.print("Set Chime");
    
    display.setCursor(30, 22*5);
    display.print("Next");

    display.setCursor(200, 22*5);
    display.print("Delete");
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

    //if(checkScreenTimeout()) return;
    if(manageLoop()) return;

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
          alarmgroup.writeToFile();
          currentAlarm.prevent = false;
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
  const char weekSymbol[7] = {'S','M','T','W','R','F','S'};
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
    stopTrack(); // TODO: check no alarm is playing
}

/* =========================== LIST DISPLAY =========================== */

void listLoop(const char* title, const int length, int top, void (*printLine)(int,int,int), bool (*clickAction)(int), void (*onHover)(int)) {

  int prevEncoderPostition = 0;
  //const int numLines = 4;
  
  timeSinceLastAction = millis();
  rotaryEncoder.setBoundaries(0, length, false);
  if(length > 4 && top > length-3) top = length-3;
  rotaryEncoder.setEncoderValue(top);
  displayList(false, top, title, printLine);
  displayMenuSelectionIndicator(0);

  while(true) {

    //if(checkScreenTimeout()) return;
    if(manageLoop()) return;

    if(rotaryEncoder.encoderChanged()) {
        timeSinceLastAction = millis();
        int val = rotaryEncoder.readEncoder();
        if(onHover != NULL) onHover(val);
        /*if(val == top && top != 0 && val < prevEncoderPostition) {*/
        if(val < top && val >= 0 && val < prevEncoderPostition) {
            prevEncoderPostition = val;
            //top--;
            top = val;
            displayList(true, top, title, printLine);
            displayMenuSelectionIndicator(0);
        } else
        /*if(val == top+4 && top != length && val > prevEncoderPostition) {*/
        if(val >= top+4 && val <= length && val > prevEncoderPostition) {
            prevEncoderPostition = val;
            //top++;
            top = val - 3;
            displayList(true, top, title, printLine);
            displayMenuSelectionIndicator(3);
        } else {
            displayMenuSelectionIndicator(val - top);
        }
    }
    
    if(rotaryEncoder.isEncoderButtonClicked()) {
      if(clickAction(rotaryEncoder.readEncoder())) return;
      else displayList(true, top, title, printLine);
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

/* =========================== MATH SNOOZE SELECTION =========================== */

void mathSnoozeLoop(Alarm& currentAlarm) {

  if(Serial) Serial.println("Displaying alarm snooze math problem");

  int term1 = random(-100,100);
  int term2 = random(-100,100);
  int solution = term1 + term2;
  int solution2 = term1 + term2 + random(-10,10);
  int solution3 = term1 + term2 + random(-10,10);
  String equation(String(term1) + String(" + ") + String(term2) + String(" ="));
  int correct = 0;
  String solutions[3];
  switch(random(0,6)) {
    case(0):
      solutions[0] = String(solution); solutions[1] = String(solution2); solutions[2] = String(solution3);
      correct = 0;
      break;
    case(1):
      solutions[1] = String(solution); solutions[0] = String(solution2); solutions[2] = String(solution3);
      correct = 1;
      break;
    case(2):
      solutions[2] = String(solution); solutions[1] = String(solution2); solutions[0] = String(solution3);
      correct = 2;
      break;
    case(3):
      solutions[0] = String(solution); solutions[2] = String(solution2); solutions[1] = String(solution3);
      correct = 0;
      break;
    case(4):
      solutions[2] = String(solution); solutions[0] = String(solution2); solutions[1] = String(solution3);
      correct = 2;
      break;
    case(5):
      solutions[1] = String(solution); solutions[2] = String(solution2); solutions[0] = String(solution3);
      correct = 1;
      break;
  }

  rotaryEncoder.setBoundaries(0, 3, false);
  rotaryEncoder.setEncoderValue(0);
  mathSnoozeDisplay(false, equation, solutions, 0);
  timeSinceLastAction = millis();

  while(true) {

    //if(checkScreenTimeout()) return;
    if(manageLoop()) return;

    if(rotaryEncoder.encoderChanged()) {
      timeSinceLastAction = millis();
      mathSnoozeDisplay(true, equation, solutions, rotaryEncoder.readEncoder());
    }
    
    if(rotaryEncoder.isEncoderButtonClicked()) {
      int val = rotaryEncoder.readEncoder();
      if(val == correct) {
        screen = clock_scr;
        currentAlarm.turnOff();
        return;
      }
    }
  }
}

void mathSnoozeDisplay(bool partial, const String& equation, const String (&solutions)[3], const int selection) {

  if(partial) display.setPartialWindow(0, 0, display.width(), display.height());
  else display.setFullWindow();

  int16_t tbx, tby; uint16_t tbw, tbh;
  display.getTextBounds(equation, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t x = ((display.width() - tbw) / 2) - tbx;
  uint16_t y = ((display.height() - tbh) / 2) - tby - 20;

  display.getTextBounds(solutions[0], 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t x1 = ((display.width()/3 - tbw) / 2) - tbx;
  uint16_t y1 = ((display.height() - tbh) / 2) - tby + 20;

  display.getTextBounds(solutions[1], 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t x2 = ((display.width()/3 - tbw) / 2) - tbx + display.width()/3;

  display.getTextBounds(solutions[2], 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t x3 = ((display.width()/3 - tbw) / 2) - tbx + 2*display.width()/3;

  int yy = y1 + 3;
  
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(x, y);
    display.print(equation);
    display.setCursor(x1, y1);
    display.print(solutions[0]);
    display.setCursor(x2, y1);
    display.print(solutions[1]);
    display.setCursor(x3, y1);
    display.print(solutions[2]);

    if(selection == 0) display.drawLine(x1,yy,x1+tbw,yy,GxEPD_BLACK);
    else if (selection == 1) display.drawLine(x2,yy,x2+tbw,yy,GxEPD_BLACK);
    else display.drawLine(x3,yy,x3+tbw,yy,GxEPD_BLACK);

  }
  while (display.nextPage());
}

/* =========================== POPUP =========================== */
// Display temporary message

void displayPopup(const char* message) {
  if(Serial) {
    Serial.print("Displaying popup: ");
    Serial.println(message);
  }
  unsigned long counter = millis();
  int16_t tbx, tby; uint16_t tbw, tbh;
  display.getTextBounds(message, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t x = ((display.width() - tbw) / 2) - tbx;
  uint16_t y = ((display.height() - tbh) / 2) - tby;
  display.setFullWindow();
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(x, y);
    display.print(message);
  }
  while (display.nextPage());
  while(millis() - counter < 10000) {
    if(manageLoop()) return;
  }
  timeSinceLastAction = millis();
}

/* =========================== ERROR =========================== */

void displayError(const char* message) {
  if(Serial) {
    Serial.print("Displaying Error: ");
    Serial.println(message);
  }
  int16_t tbx, tby; uint16_t tbw, tbh;
  display.getTextBounds(message, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t x = ((display.width() - tbw) / 2) - tbx;
  uint16_t y = (display.height() - tbh);
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.drawBitmap(display.width()/2 - 48/2, 20, error_icon, 48, 48, GxEPD_BLACK);
    display.setCursor(x,y);
    display.print(message);
  }
  while (display.nextPage());
  while(true) {
    // TODO: prevent screen timeout in this situation
    if(manageLoop() || WiFi.status() == WL_CONNECTED) return;
  }
}

/* =========================== AP MODE MESSAGE =========================== */

void displayApMode(const char* ipAddr) {
  if(Serial) Serial.print("Displaying AP mode message");
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    displayTitle("Wi-Fi Connection Failed");
    display.drawBitmap(10, 24 + (display.height()-24)/2 - 48/2, error_wifi_icon, 48, 48, GxEPD_BLACK);

    display.setCursor(68,48);
    display.print("1. Connect to alarm");
    display.setCursor(100,66);
    display.print("clock over wifi");

    display.setCursor(68,96);
    display.print("2. Go to");
    display.setCursor(100,114);
    display.print(ipAddr);
  }
  while (display.nextPage());
  /*while(true) {
    // TODO: prevent screen timeout in this situation
    if(manageLoop() || WiFi.status() == WL_CONNECTED) return;
  }*/
  while(true);
}

/*

Wi-Fi Connection Failed
1. Connect to alarm clock from phone wifi
2. Go to 'ip-address'

*/