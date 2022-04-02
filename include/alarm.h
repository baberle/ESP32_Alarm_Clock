#ifndef __ALARM_H
#define __ALARM_H

#include "alarmplayer.h"

struct Alarm 
{
    int hour;
    int minute;
    bool day[7];
    bool active;
    bool snooze;
    AlarmPlayer ap;
    Alarm();
    void checkAlarm(tm &timeinfo);
    void increaseTime(int amt);
    void decreaseTime(int amt);
    void setTime(int amt);
    void toString(bool militaryTime, char* timeString); // char array should be at least size 10
    void toDayString(char* dayString); // TODO: these should be const
    void reset();
};

#endif