#include "alarmset.h"

/*bool AlarmSet::addAlarm() {
    if(numSetAlarms < 10) {
        alarms[numSetAlarms].reset();
        numSetAlarms++;
        return true;
    } else {
        return false;
    }
}

bool AlarmSet::deleteAlarm(int idx) {
    if(idx < 0 || idx > numSetAlarms-1) return false;
    return false;
    // this is kind of hard
}

// Find the alarm that will go off next
int AlarmSet::getNextAlarm() const {

  int nxtAlarm = -1;
  int nearestHour = 24;
  int nearestMin = 61;
  int daysAway = 7;

  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return -1;
  }

  //Serial.print("Today is day ");
  //Serial.println(timeinfo.tm_wday);

  for(int i = 0; i < numSetAlarms; i++) {

    //Serial.print("--- Alarm ");
    //Serial.println(i);

    if(alarms[i].active) {

      for(int j = 0; j < 7; j++) {

        int day = (j+timeinfo.tm_wday) % 7;

        //Serial.print("Day: ");
        //Serial.println(day);

        if(!alarms[i].day[day]) continue;

        if(j <= daysAway) {
          if(day == timeinfo.tm_wday && (timeinfo.tm_hour > alarms[i].hour || (timeinfo.tm_hour == alarms[i].hour && timeinfo.tm_min > alarms[i].minute))) {
            continue;
          } else {
            if(j != daysAway || nearestHour > alarms[i].hour || (nearestHour == alarms[i].hour && nearestMin > alarms[i].minute)) {
              daysAway = j;
              nearestHour = alarms[i].hour;
              nearestMin = alarms[i].minute;
              nxtAlarm = i;
              //Serial.print(daysAway);
              //Serial.println(" days from now");
            }
          }
        } else {
          break;
        }
      }
    }
  }

  return nxtAlarm;

}

Alarm* AlarmSet::checkAllAlarms(tm &timeinfo) {
    // TODO: does this work of disabled alarms?
    for(int i = 0; i < numSetAlarms; i++) {
        if(alarms[i].checkAlarm(timeinfo)) {
          return &alarms[i];
        }
    }
    return nullptr;
}*/

/*bool AlarmSet::addAlarm() {
  if(alarms.isFull()) {
    return false;
  } else {
    Alarm* newAlarm = new Alarm();
    alarms.push_back(newAlarm);
    return true;
  }
}

void AlarmSet::getAlarm(int idx) {
  return alarms.at(idx);
}

void AlarmSet::deleteAlarm(int idx) {
  alarms.remove(idx);
}

Alarm* AlarmSet::checkAllAlarms(tm &timeinfo) {
  for(int i = 0; i < alarms.size(); i++) {
    if(alarms.at(i)->checkAlarm(timeinfo)) {
      return &alarms.at(i);
    }
  }
  return nullptr;
}*/
