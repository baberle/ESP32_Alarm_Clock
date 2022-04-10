#ifndef __ALARMSET_H
#define __ALARMSET_H

#include "alarm.h"

/*struct AlarmSet {
  Alarm alarms[10];
  int numAlarms = 10;
  int numSetAlarms = 0;
  bool addAlarm();
  bool deleteAlarm(int idx);
  int getNextAlarm() const;
  Alarm* checkAllAlarms(tm &timeinfo);
};*/

/*template<typename T>
class Group {
  T* _arr;
  int _size;
  int _maxSize;
  public:
  Group() = delete;
  Group(int size) {
    _arr = new T(size);
    _size = 0;
    _maxSize = size;
  }
  ~Group() { delete[] _arr; }
  int size() const { return _size; }
  int maxSize() const { return _maxSize; }
  bool isFull() const { return _size == _maxSize; }
  T* at(int idx) {
    if(idx < 0 || idx >= _size) {
      return nullptr;
    } else {
      return &_arr[idx];
    }
  }
  void remove(int idx) {
    if(idx >= 0 || idx < _size) {
      delete &_arr[idx];
      for(int i = idx+1; i < _size; i++) {
        _arr[i-1] = _arr[i];
      }
      _size = _size - 1;
    }
  }
  void push_back(T* item) {
    if(_size < _maxSize) {
      _arr[_size-1] = &item;
    }
  }
  // TODO: array contents are not deleted
};

class AlarmSet {
  Group<Alarm> alarms;
  public:
  Alarmset() { alarms = new Group(10); }
  bool addAlarm();
  void deleteAlarm(int idx);
  void getAlarm(int idx);
  Alarm* checkAllAlarms(tm &timeinfo);
};*/

class AlarmGroup {
  Alarm* _arr[10];
  int _size = 0;
  int _maxSize = 10;
  public:
  //AlarmGroup();
  ~AlarmGroup() { 
    for(int i = 0; i < _size; i++) {
      delete _arr[i];
    }
  }
  int size() const { return _size; }
  int maxSize() const { return _maxSize; }
  bool isFull() const { return _size == _maxSize; }
  Alarm* at(int idx) {
    if(idx < 0 || idx >= _size) {
      return nullptr;
    } else {
      return _arr[idx];
    }
  }
  void remove(int idx) {
    if(idx >= 0 || idx < _size) {
      delete _arr[idx];
      for(int i = idx+1; i < _size; i++) {
        _arr[i-1] = _arr[i];
      }
      _size = _size - 1;
    }
  }
  Alarm* add() {
    if(_size < _maxSize) {
      Serial.println("Added new alarm");
      _arr[_size] = new Alarm();
      _size++;
      return _arr[_size-1];
    } else {
      Serial.println("Could not add new alarm");
      return nullptr;
    }
  }
  Alarm* checkAll(tm &timeinfo) {
    for(int i = 0; i < _size; i++) {
      if(_arr[i]->checkAlarm(timeinfo)) {
        return _arr[i];
      }
    }
    return nullptr;
  }
};

#endif