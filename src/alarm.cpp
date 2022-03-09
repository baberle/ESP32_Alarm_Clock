#include "alarm.h"

Alarm::Alarm() {
    hour = 12;
    minute = 0;
    for(int i = 0; i < 7; i++) day[i] = true;
    active  = true;
    ap.track = 1;
}

// TODO: this should only go off once a minute even if the person hits snooze before the minute is up (keep snooze value in alarm player)
void Alarm::checkAlarm(tm &timeinfo) {
    if(active) {
        if(!ap.alarmEnabled) {
            if(day[timeinfo.tm_wday]) {
                if(timeinfo.tm_hour == hour && timeinfo.tm_min == minute) {
                    ap.startAlarmPlayer();
                }
            }
        } 
        else {
            ap.manageAlarmPlayer();
        }
    }
} 

void Alarm::increaseTime(int amt) {
  int timeToNextHour = 60 - minute;
  if(amt < timeToNextHour) {
    minute += amt; 
  } else {
    if(hour == 24) {
      hour = 0;
    } else {
      hour++;
    }
    minute = amt - timeToNextHour;    
  }
}

void Alarm::decreaseTime(int amt) {
  if(amt < minute) {
    minute -= amt; 
  } else {
    if(hour == 0) {
      hour = 24;
    } else {
      hour--;
    }
    minute = 59 - (amt - minute);    
  }
}