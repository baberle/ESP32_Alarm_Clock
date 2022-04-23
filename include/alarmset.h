#ifndef __ALARMSET_H
#define __ALARMSET_H

#include "alarm.h"
#include <SPIFFS.h>
#include <string.h>

class AlarmGroup {
  Alarm* _arr[10];
  int _size = 0;
  int _maxSize = 10;
  Alarm* goingOff = nullptr;
  void writeLine(const Alarm* al, File& file) const;
  void readLine(String& line);
  public:
  ~AlarmGroup();

  int size() const { return _size; }
  int maxSize() const { return _maxSize; }
  bool isFull() const { return _size == _maxSize; }
  bool isEmpty() const { return _size == 0; }

  Alarm* at(int idx);
  void remove(int idx);
  Alarm* add();
  Alarm* checkAll(tm &timeinfo) const;

  void hitOff();
  void hitSnooze();
  //Alarm* alarmIsGoingOff();
  
  void writeToFile() const;
  void printFile(fs::FS &fs);
  void readFile();
};

#endif