#include "alarm.h"

Alarm::Alarm() {
    hour = 12;
    minute = 0;
    for(int i = 0; i < 7; i++) day[i] = true;
    active  = true;
    ap.track = 1;
}

void Alarm::checkAlarm(tm &timeinfo) {
    if(active && goingOff(timeinfo) && !ap.alarmEnabled) {
        ap.startAlarmPlayer(); // TODO: put this part of the code in going off
    }
    if(ap.alarmEnabled) ap.manageAlarmPlayer();
}

// TODO: this should only go off once a minute even if the person hits snooze before the minute is up (keep last minute called in memory)
//              call only when the minute changes?
//              doesn't work if snoozed at 30 second mark and alarm has to go off at 30 second mark
// TODO: Perhaps combine with check alarm
bool Alarm::goingOff(tm &timeinfo) const
{
    bool alarmMatchesCurrentTime = false;
    if(active) {
        if(day[timeinfo.tm_wday]) {
            if(timeinfo.tm_hour == hour && timeinfo.tm_min == minute) {
                alarmMatchesCurrentTime = true;
            }
        }
    }
    return alarmMatchesCurrentTime;
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