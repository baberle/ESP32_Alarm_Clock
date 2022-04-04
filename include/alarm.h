#ifndef __ALARM_H
#define __ALARM_H

#include "alarmplayer.h"

enum SnoozeType {
    on,
    off,
    math
};

struct Alarm 
{
    int hour;
    int minute;
    bool day[7];
    bool active;
    bool alreadyPlayed;
    SnoozeType snooze;
    AlarmPlayer ap;
    Alarm();
    void reset();
    bool checkAlarm(tm &timeinfo);
    void snoozeAlarm();
    void turnOff();
    void setTime(int amt);
    // char array should be at least size 10
    void toString(bool militaryTime, char* timeString) const;
    void toDayString(char* dayString) const;
};

#endif