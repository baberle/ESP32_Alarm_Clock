#include "alarmclock.h"

int prevMinute = 0;
enum Change {none, full, partial};
bool militaryTime = false;

enum Screen {clock_scr, main_menu_scr, alarms_scr, timezone_scr};
Screen screen = timezone_scr;
unsigned long timeSinceLastAction = millis();

Alarm al;

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

/*const int timeZoneOffset[24] = {

};*/


const char* hostname = "ESP32 Alarm Clock";
const bool WiFiEnabled = true;

AiEsp32RotaryEncoder rotaryEncoder = AiEsp32RotaryEncoder(ROTARY_ENCODER_A_PIN, ROTARY_ENCODER_B_PIN, ROTARY_ENCODER_BUTTON_PIN, ROTARY_ENCODER_VCC_PIN, ROTARY_ENCODER_STEPS);

GxEPD2_BW<GxEPD2_290, GxEPD2_290::HEIGHT> display(GxEPD2_290(EINK_CS, EINK_DC, EINK_RST, EINK_BUSY)); // GDEH029A1 128x296


/*#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;*/



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

  /*SerialBT.begin("ESP32test");
  Serial.println("The device started, now you can pair it with bluetooth!");*/

  setupWiFi();
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  setupDFPlayer();
  al.hour = 18;
  al.minute = 39;

  char timeString[10];
  al.toString(false, timeString);
  Serial.print("timeString: "); Serial.println(timeString);
  al.toString(true, timeString);
  Serial.print("timeString: "); Serial.println(timeString);


  rotaryEncoder.begin();
  rotaryEncoder.setup(readEncoderISR);
  rotaryEncoder.setBoundaries(0, 1000, false); //minValue, maxValue, circleValues true|false (when max go to min and vice versa)
  rotaryEncoder.setAcceleration(250); 
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
  
  manageLoop();
  
  if (rotaryEncoder.encoderChanged())
  {
      Serial.println(rotaryEncoder.readEncoder());
  }
  if (rotaryEncoder.isEncoderButtonClicked())
  {
      Serial.println("button pressed");
  }

  /*if (Serial.available()) {
    SerialBT.write(Serial.read());
  }
  if (SerialBT.available()) {
    Serial.write(SerialBT.read());
  }
  delay(20);*/

  switch(screen) {
    case clock_scr:
      mainTimeDisplayLoop();
      break;
    case main_menu_scr:
      mainMenuLoop();
      break;
    case alarms_scr:
      alarmsLoop();
      break;
    case timezone_scr:
      timezoneLoop();
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
  al.checkAlarm(timeinfo);
}


void drawPairing() {

  const char pairing[] = "Pairing ...";
  int16_t tbx, tby; uint16_t tbw, tbh;
  display.getTextBounds(pairing, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t x = ((display.width() - tbw) / 2) - tbx;
  uint16_t y = ((display.height() - tbh) / 2) - tby;

  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(x,y);
    display.print(pairing);
    drawWiFiIcon();
  }
  while (display.nextPage());

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
  if(al.active) {
    display.drawBitmap(296-24-24-2, 1, alarm_icon, 24, 24, GxEPD_BLACK); 
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
          break;
        case 1:
          break;
        case 2:
          //screen = main_settings_scr;
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


//int numAlarms = 6;
//int numSetAlarms = 3;
//Alarm alarms[10];

AlarmSet alarmset;

void testSetup() {

  alarmset.addAlarm();
  alarmset.addAlarm();
  alarmset.addAlarm();
  alarmset.addAlarm();

  alarmset.alarms[0].active = true;
  alarmset.alarms[0].hour = 11;
  alarmset.alarms[0].minute = 52;
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

void alarmsLoop() {

  if(Serial) Serial.println("Displaying alarms menu");

  testSetup();
  int top = 0;
  int prevEncoderPostition = 0; // Probably already have this somewhere

  //Serial.println(getNextAlarm());
  
  timeSinceLastAction = millis();
  rotaryEncoder.setBoundaries(0, alarmset.numSetAlarms+2, false);
  rotaryEncoder.setEncoderValue(0);
  displayAlarms(false, top);
  displayMenuSelectionIndicator(0);

  while(true) {

    if(rotaryEncoder.encoderChanged()) {
      timeSinceLastAction = millis();
      int val = rotaryEncoder.readEncoder();
      Serial.print("Top: ");
      Serial.print(top);
      Serial.print(" Val: ");
      Serial.println(val);
      if(val == top && top != 0 && val < prevEncoderPostition) {
        prevEncoderPostition = val;
        top--;
        displayAlarms(true, top);
        displayMenuSelectionIndicator(0);
      } else
      if(val == top+4 && top != alarmset.numSetAlarms-2 && val > prevEncoderPostition) {
        prevEncoderPostition = val;
        top++;
        displayAlarms(true, top);
        displayMenuSelectionIndicator(3);
      } else {
        displayMenuSelectionIndicator(val - top);
      }
    }
    if(checkScreenTimeout()) return;
    
    if(rotaryEncoder.isEncoderButtonClicked()) {
      int val = rotaryEncoder.readEncoder();
      if(val < alarmset.numSetAlarms) {
        // go to alarm setsing screen for alarm # val
        Serial.println("Go into alarm");
      } else 
      if(val == alarmset.numSetAlarms) {
        // Add new alarm logic
        // go to alarm setting screen if room for another alarm
        Serial.println("Making new alarm");
        alarmset.addAlarm();
      } else 
      if(val == alarmset.numSetAlarms+1) {
        screen = main_menu_scr;
        return;
      }
    }
  }
}


void displayAlarmLine(Alarm& alarm, int lineNum) {

  int lineHeight = 22*(lineNum+2);

  char timeString[10];
  alarm.toString(militaryTime, timeString);
  display.setCursor(30, lineHeight);
  display.print(timeString);

  display.drawLine(130, lineHeight, 130, lineHeight-22, GxEPD_BLACK);

  char dayString[10];
  alarm.toDayString(dayString);
  int16_t tbx, tby; uint16_t tbw, tbh;
  display.getTextBounds(dayString, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t x = ((120 - tbw) / 2) - tbx;
  display.setCursor(130+x, lineHeight);
  display.print(dayString);

  display.drawLine(250, lineHeight, 250, lineHeight-22, GxEPD_BLACK);

  display.setCursor(260, lineHeight);
  display.print(alarm.active ? "ON" : "OFF");
}

// TODO: add partial refresh option
void displayAlarms(bool partial, int topA) {

  if(!partial) display.setFullWindow();
  else display.setPartialWindow(0, 24, display.width(), display.height()-22);
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);

    if(!partial) displayTitle("Alarms");

    int row = 0;
    for(int i = topA; i < topA+4; i++) {
      if(i < alarmset.numSetAlarms) {
        displayAlarmLine(alarmset.alarms[i], row);
      } else {
        if(i == alarmset.numSetAlarms) {
          int lineHeight = 22*(row+1);
          display.drawBitmap(30, lineHeight+8, plus_outline, 18, 18, GxEPD_BLACK);
          display.setCursor(30+18+4, lineHeight+22);
          display.print("New Alarm");
        } else 
        if(i == alarmset.numSetAlarms+1) {
          display.setCursor(30, 22*5);
          display.print("Exit");
        }
      }
      row++;
    }

  }
  while (display.nextPage());
}



/* =========================== CHOOSE TIMEZONES =========================== */



void timezoneLoop() {

  if(Serial) Serial.println("Displaying timezone menu");

  int top = 24;
  int prevEncoderPostition = 24;
  
  timeSinceLastAction = millis();
  rotaryEncoder.setBoundaries(0, 30, false);
  rotaryEncoder.setEncoderValue(24);
  displayTimezones(false, top);
  displayMenuSelectionIndicator(0);

  while(true) {

    if(checkScreenTimeout()) return;

    if(rotaryEncoder.encoderChanged()) {
      timeSinceLastAction = millis();
      int val = rotaryEncoder.readEncoder();
      if(val == top && top != 0 && val < prevEncoderPostition) {
        prevEncoderPostition = val;
        // TODO: should increade by possibly more then once because the library (but make sure it doesn't overstep)
        top--;
        displayTimezones(true, top);
        displayMenuSelectionIndicator(0);
      } else
      if(val == top+4 && top != 30 && val > prevEncoderPostition) {
        prevEncoderPostition = val;
        top++;
        displayTimezones(true, top);
        displayMenuSelectionIndicator(3);
      } else {
        displayMenuSelectionIndicator(val - top);
      }
    }
    
    if(rotaryEncoder.isEncoderButtonClicked()) {
      //int val = rotaryEncoder.readEncoder();
      // TODO: change timezone offset
      // TODO: This should go to the main menu screen which I did not add yet
      screen = main_menu_scr;
      return;
    }

  }
}


void displayTimezones(bool partial, int topA) {

  if(partial) display.setPartialWindow(0, 24, display.width(), display.height()-22);
  else display.setFullWindow();

  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);

    if(!partial) displayTitle("Alarms");

    int row = 0;
    for(int i = topA; i < topA+4; i++) {
      // TODO: I think I should be able to fit 5 in here
      // TODO: add "time" after every zone name
      if(i > 30) Serial.print("Out of bounds");
      int lineHeight = 22*(row+2);
      display.setCursor(30, lineHeight);
      display.print(timeZoneDescription[i]);
      row++;
    }

  }
  while (display.nextPage());
}