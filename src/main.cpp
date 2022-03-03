#include <Arduino.h>
#include "alarmplayer.h"

AlarmPlayer ap(4);

void setup() {
  Serial.begin(115200);
  setupDFPlayer();
  delay(500);
  ap.startAlarmPlayer();
}

void loop() 
{
  manageDFPlayer();
  ap.manageAlarmPlayer();
}