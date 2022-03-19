#ifndef __ALARMSET_H
#define __ALARMSET_H

#include "alarm.h"

struct AlarmSet {
  Alarm alarms[10];
  int numAlarms = 10;
  int numSetAlarms = 0;
  bool addAlarm();
  bool deleteAlarm(int idx);
  int getNextAlarm() const;
  void checkAllAlarms(tm &timeinfo);
};

#endif