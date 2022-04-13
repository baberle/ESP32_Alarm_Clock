#include "alarm.h"

Alarm::Alarm() {
    hour = 12;
    minute = 0;
    for(int i = 0; i < 7; i++) day[i] = true;
    active  = false;
    snooze = on;
    alreadyPlayed = false;
    ap.track = 1;
}

void Alarm::reset() {
  hour = 12;
  minute = 0;
  for(int i = 0; i < 7; i++) day[i] = true;
  active  = true;
  snooze = on;
  alreadyPlayed = false;
  ap.track = 1;
}

// TODO: this should only go off once a minute even if the person hits snooze before the minute is up (keep snooze value in alarm player)
bool Alarm::checkAlarm(tm &timeinfo) {
  if(active) {
    if(alreadyPlayed) {
      if(timeinfo.tm_min == minute+1) alreadyPlayed = false;
    } else
    if(!ap.alarmEnabled) {
        if(day[timeinfo.tm_wday]) {
            if(timeinfo.tm_hour == hour && timeinfo.tm_min == minute) {
                Serial.println("Starting Alarm");
                printThis();
                ap.startAlarmPlayer();
                return true;
                // TODO: do checking if other alarms are playing here
            }
        }
    } 
    ap.manageAlarmPlayer();
  }
  return false;
} 

void Alarm::turnOff() {
  ap.stopAlarmPlayer();
  alreadyPlayed = true;
}

void Alarm::setTime(int amt) {
  hour = amt / 60;
  minute = amt % 60;
  alreadyPlayed = false;
}

// TODO: ensure it works for edge cases
void Alarm::toString(bool militaryTime, char* timeString) const {

  if(militaryTime) {
    timeString[0] = hour/10 + '0';
    timeString[1] = hour%10 + '0';
  } else {
    int newHour = hour % 12;
    timeString[0] = newHour/10 + '0';
    if(timeString[0] == '0') timeString[0] = ' ';
    timeString[1] = newHour%10 + '0';
    if(timeString[1] == '0') {
      timeString[0] = '1';
      timeString[1] = '2';
    }
  }

  timeString[2] = ':';

  timeString[3] = minute/10 + '0';
  timeString[4] = minute%10 + '0';

  timeString[5] = ' ';

  if(militaryTime) {
    timeString[6] = ' ';
    timeString[7] = ' ';
  } else {
    if(hour < 12) {
      timeString[6] = 'A';
    } else {
      timeString[6] = 'P';
    }
    timeString[7] = 'M';
  }

  timeString[8] = '\0';

}

void Alarm::toDayString(char* dayString) const {

  const char days[] = {'S','M','T','W','R','F','S'};

  bool allOn = true;
  bool allOff = true;
  bool weekdayOnly = true;
  bool weekendOnly = true;

  for(int dayNum = 0; dayNum < 7; dayNum++) {
    if(!day[dayNum]) allOn = false;
    if(day[dayNum]) allOff = false;
    if(((dayNum == 0 || dayNum == 6) &&  day[dayNum]) ||
       ((dayNum >  0 && dayNum <  6) && !day[dayNum])) weekdayOnly = false;
    if((!(dayNum == 0 || dayNum == 6) &&  day[dayNum]) ||
        ((dayNum == 0 || dayNum == 6) && !day[dayNum])) weekendOnly = false;
  }

  if(allOn) {
    strcpy(dayString, "All Days");
  } else
  if(allOff) {
    strcpy(dayString, "None");
  } else
  if(weekdayOnly) {
    strcpy(dayString, "Weekdays");
  } else
  if(weekendOnly) {
    strcpy(dayString, "Weekends");
  } else {
    int ctr = 0;
    for(int dayNum = 0; dayNum < 7; dayNum++) {
      if(day[dayNum]) {
        dayString[ctr] = days[dayNum];
        ctr++;
      }
    }
    dayString[ctr]= '\0';
  }

}

void Alarm::printThis() const {
  Serial.print("Active: ");
  Serial.println((active ? "true" : "false"));
  Serial.print("Already Played: ");
  Serial.println((alreadyPlayed ? "true" : "false"));
  Serial.println("Snooze: ");
  Serial.print(snooze);
  Serial.print("Time: ");
  Serial.print(hour);
  Serial.print(":");
  Serial.println(minute);
  Serial.print("Days: ");
  for(int i = 0; i < 7; i++) Serial.print(day[i]);
  Serial.println("");
}