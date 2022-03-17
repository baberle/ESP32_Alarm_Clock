#ifndef __ALARM_H
#define __ALARM_H

#include "alarmplayer.h"

struct Alarm 
{
    int hour;
    int minute;
    bool day[7];
    bool active;
    AlarmPlayer ap;
    Alarm();
    void checkAlarm(tm &timeinfo);
    void increaseTime(int amt);
    void decreaseTime(int amt);
    void toString(bool militaryTime, char* timeString); // char array should be at least size 10
};

#endif