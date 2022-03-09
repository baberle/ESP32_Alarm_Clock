#ifndef __ALARMPLAYER_H
#define __ALARMPLAYER_H

#include <Arduino.h>
#include <DFMiniMp3.h>

void setupDFPlayer(int alarmTimeout = 60000, int snoozeTime = 600000);
void manageDFPlayer();

struct AlarmPlayer {
  int track;
  int volume;
  bool alarmEnabled; // true while the alarm is playing
  bool snoozed;
  unsigned long startTime;
  unsigned long volIncrease;
  AlarmPlayer();
  AlarmPlayer(const int _track);
  void startAlarmPlayer();
  void stopAlarmPlayer();
  void snoozeAlarmPlayer();
  void manageAlarmPlayer();
};

#endif