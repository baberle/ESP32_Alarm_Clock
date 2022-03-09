#include "alarmplayer.h"

class Mp3Notify; 
typedef DFMiniMp3<HardwareSerial, Mp3Notify> DfMp3;

DfMp3 dfmp3(Serial1);

/*
TODO: Snooze
TODO: volume ramp
TODO: detect if sd card is inserted
TODO: manage collisions if two alarms playing at the same time
*/

// Each soundbyte should be 1 minute long and ramp from 0 to 15 volume in 10 seconds
const char *fileTitles[4] = {
  "Star Wars",
  "Indiana Jones",
  "Tintin",
  "The Cowboys"
};

int alarmTimeout = 60000;
int snoozeTime = 600000;

AlarmPlayer::AlarmPlayer(const int _track) {
  track = _track;
}

AlarmPlayer::AlarmPlayer() {
  track = 1;
}

void AlarmPlayer::startAlarmPlayer() {
  volIncrease = millis();
  startTime = millis();
  alarmEnabled = true;
  dfmp3.playMp3FolderTrack(track);
  dfmp3.setRepeatPlayCurrentTrack(true);
  dfmp3.setVolume(0);
}

void AlarmPlayer::stopAlarmPlayer() {
  alarmEnabled = false;
  dfmp3.stop();
}

void AlarmPlayer::manageAlarmPlayer() {
  if(alarmEnabled) {
    if(millis() - startTime > alarmTimeout) {
      alarmEnabled = false;
      dfmp3.stop();
    } else
    if(millis() - volIncrease > 200 && dfmp3.getVolume() < 15) {
      dfmp3.increaseVolume();
      volIncrease = millis();
      uint16_t volume = dfmp3.getVolume();
      Serial.print("Volume: ");
      Serial.println(volume);
    }
  }
}

class Mp3Notify
{
public:
  static void PrintlnSourceAction(DfMp3_PlaySources source, const char* action)
  {
    if (source & DfMp3_PlaySources_Sd) 
    {
        Serial.print("SD Card, ");
    }
    if (source & DfMp3_PlaySources_Usb) 
    {
        Serial.print("USB Disk, ");
    }
    if (source & DfMp3_PlaySources_Flash) 
    {
        Serial.print("Flash, ");
    }
    Serial.println(action);
  }
  static void OnError(DfMp3& mp3, uint16_t errorCode)
  {
    // see DfMp3_Error for code meaning
    Serial.println();
    Serial.print("Com Error ");
    Serial.println(errorCode);
  }
  static void OnPlayFinished(DfMp3& mp3, DfMp3_PlaySources source, uint16_t track)
  {
    Serial.print("Play finished for #");
    Serial.println(track);  
  }
  static void OnPlaySourceOnline(DfMp3& mp3, DfMp3_PlaySources source)
  {
    PrintlnSourceAction(source, "online");
  }
  static void OnPlaySourceInserted(DfMp3& mp3, DfMp3_PlaySources source)
  {
    PrintlnSourceAction(source, "inserted");
  }
  static void OnPlaySourceRemoved(DfMp3& mp3, DfMp3_PlaySources source)
  {
    PrintlnSourceAction(source, "removed");
  }
};

void setupDFPlayer(int alarmTimeout, int snoozeTime) {
    dfmp3.begin();
    dfmp3.setVolume(30);
    if(Serial) {
        Serial.println("initializing...");
        uint16_t volume = dfmp3.getVolume();
        Serial.print("volume ");
        Serial.println(volume);
        uint16_t count = dfmp3.getTotalTrackCount(DfMp3_PlaySource_Sd);
        Serial.print("files ");
        Serial.println(count);
        Serial.println("starting...");
    }
}

void manageDFPlayer() {
    dfmp3.loop();
}