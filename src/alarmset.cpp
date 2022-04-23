#include "alarmset.h"

// Deallocate existing alarms on the heap
AlarmGroup::~AlarmGroup() {
  for(int i = 0; i < _size; i++) {
    delete _arr[i];
  }
}

// Get a reference to a specific alarm
Alarm* AlarmGroup::at(int idx) {
  if(idx < 0 || idx >= _size) return nullptr;
  else return _arr[idx];
} 

// Adds a new alarm. Returns null if no more room.
Alarm* AlarmGroup::add() {
  if(_size < _maxSize) {
    _arr[_size] = new Alarm();
    _size++;
    return _arr[_size-1];
  } else {
    return nullptr;
  }
}

// Checks if any alarms are going off and returns that alarm
Alarm* AlarmGroup::checkAll(tm &timeinfo) const {
  for(int i = 0; i < _size; i++) {
    if(_arr[i]->checkAlarm(timeinfo)) return _arr[i];
  }
  return nullptr;
}

// Remove a specific alarm (shifts all elements over to remove the gap)
void AlarmGroup::remove(int idx) {
  if(idx >= 0 && idx < _size) {
    delete _arr[idx];
    for(int i = idx+1; i < _size; i++) _arr[i-1] = _arr[i];
    _size = _size - 1;
  }
}

// Turns off any alarms in the group that are going off
void AlarmGroup::hitOff() {
  Serial.println("Turning off alarms");
  for(int i = 0; i < _size; i++) {
    if(_arr[i]->ap.alarmEnabled  && _arr[i]->snooze != math) _arr[i]->turnOff();
  }
}

// Snoozes any alarms in the group that are going off
void AlarmGroup::hitSnooze() {
  Serial.println("Snoozing alarms");
  for(int i = 0; i < _size; i++) {
    if(_arr[i]->ap.alarmEnabled && _arr[i]->snooze == on) _arr[i]->ap.snoozeAlarmPlayer();
  }
}

// Write an alarm to the file on a single line
void AlarmGroup::writeLine(const Alarm* al, File& file) const {
  // TODO: Should I print the end of file indicator?
  //enabled,active,hour,minute,M,T,W,R,F,S,track,snooze
  file.print('1'); // enabled
  file.print(',');
  file.print((al->active ? '1' : '0'));
  file.print(',');
  file.print(al->hour);
  file.print(',');
  file.print(al->minute);
  file.print(',');
  file.print(al->day[0]);
  file.print(',');
  file.print(al->day[1]);
  file.print(',');
  file.print(al->day[2]);
  file.print(',');
  file.print(al->day[3]);
  file.print(',');
  file.print(al->day[4]);
  file.print(',');
  file.print(al->day[5]);
  file.print(',');
  file.print(al->day[6]);
  file.print(',');
  file.print(al->ap.track);
  file.print(',');
  file.print((al->snooze == on ? '1' : (al->snooze == off ? '0' : '2')));
  file.print('\n');
}

// Write all alarms in the alarmgroup out to a file in csv format
void AlarmGroup::writeToFile() const {

  Serial.println("Writing to file alarms.csv");

  if (!SPIFFS.begin(true)) Serial.println("Error mounting SPIFFS");
  else Serial.println("SPIFFS mounted successfully");

  File file = SPIFFS.open("/alarms.csv", FILE_WRITE);
  if(!file) {
    Serial.println("- failed to open file for writing");
    return;
  }

  file.print("enabled,active,hour,minute,M,T,W,R,F,S,track,snooze\n");
  
  int i = 0;
  for(; i < _size; i++) {
    writeLine(_arr[i], file);
  }

  for(; i < _maxSize; i++) {
    file.print("0\n");
  }

}

// Prints out the contents of the alarmgroup file to the serial monitor
void AlarmGroup::printFile(fs::FS &fs) {
  Serial.printf("Reading file: /alarms.csv\r\n");
  if (!SPIFFS.begin(true)) Serial.println("Error mounting SPIFFS");
  else Serial.println("SPIFFS mounted successfully");
  File file = fs.open("/alarms.csv");
  if(!file || file.isDirectory()){
    Serial.println("- failed to open file for reading");
    return;
  }
  while(file.available()){
    Serial.write(file.read());  
  }
}

// Reads in single line from the file and converts it to an alarm
void AlarmGroup::readLine(String& line) {
  //enabled,active,hour,minute,M,T,W,R,F,S,track,snooze
  /*auto popNextProperty = [](String& line) {
    Serial.print("Line: ");
    Serial.print(line);
    String out = "";
    for(char c : line) {
        Serial.print(" char-");
        Serial.print(c);
      if(c == ',' || c == '\n') {
        line.remove(0,1);
        break;
      } else {
        out = out + String(c);
        line.remove(0,1);
      }
    }
    Serial.print(" Got: ");
    Serial.println(out);
    return out.toInt();
  };*/
  auto popNextProperty = [](String& line) {
    //Serial.print("Line: ");
    //Serial.print(line);
    String out = "";
    while(!line.isEmpty()) {
      char c = line[0];
      //Serial.printf(" char-%c", c);
      if(c == ',' || c == '\n') {
        line.remove(0,1);
        break;
      } else {
        out = out + String(c);
        line.remove(0,1);
      }
    }
    //Serial.print(" Got: ");
    //Serial.println(out);
    return out.toInt();
  };
  if(popNextProperty(line) == 0) return;
  Alarm* al = add();
  al->active = popNextProperty(line);
  al->hour = popNextProperty(line);
  al->minute = popNextProperty(line);
  al->day[0] = popNextProperty(line);
  al->day[1] = popNextProperty(line);
  al->day[2] = popNextProperty(line);
  al->day[3] = popNextProperty(line);
  al->day[4] = popNextProperty(line);
  al->day[5] = popNextProperty(line);
  al->day[6] = popNextProperty(line);
  al->ap.track = popNextProperty(line);
  switch(popNextProperty(line)) {
    case(1): al->snooze = on; break;
    case(0): al->snooze = off; break;
    case(2): al->snooze = math; break;
  }
}

// Reads the alarms file and converts it to alarms in the alarmgroup
void AlarmGroup::readFile() {

  Serial.println("Reading file /alarms.csv");

  if (!SPIFFS.begin(true)) Serial.println("Error mounting SPIFFS");
  else Serial.println("SPIFFS mounted successfully");

  File file = SPIFFS.open("/alarms.csv", FILE_READ);
  if(!file) {
    Serial.println("- failed to open file for reading");
    return;
  }
  
  // Clear all existing alarms
  while(!isEmpty()) remove(_size - 1);
  
  // Add all alarms stored in file
  String test = file.readStringUntil('\n'); // remove title line
  while(file.available()) {
    String line = file.readStringUntil('\n');
    readLine(line);
  }

  // TODO: close spiffs?
  // TODO: test what happens when deleting time
}