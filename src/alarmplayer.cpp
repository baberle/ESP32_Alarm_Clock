#include "alarmplayer.h"

class Mp3Notify; 
typedef DFMiniMp3<HardwareSerial, Mp3Notify> DfMp3;

DfMp3 dfmp3(Serial1);

/*
TODO: detect if sd card is inserted
TODO: manage collisions if two alarms playing at the same time
*/

// Each soundbyte should be 1 minute long and ramp from 0 to 15 volume in 10 seconds
/*const char *fileTitles[4] = {
  "Star Wars",
  "Indiana Jones",
  "Tintin",
  "The Cowboys"
};*/

int alarmTimeout = 60000;
int snoozeTime = 600000;
bool alarmIsPlaying = false;

AlarmPlayer::AlarmPlayer(const int _track) {
  track = _track;
}

AlarmPlayer::AlarmPlayer() {
  track = 1;
}

void AlarmPlayer::startAlarmPlayer() {
  if(Serial) Serial.println("Starting alarm player");
  volIncrease = millis();
  startTime = millis();
  alarmEnabled = true;
  snoozed = false;
  volume = 0;
  dfmp3.setVolume(0);
  dfmp3.playMp3FolderTrack(track);
  dfmp3.playMp3FolderTrack(track); // HACK:
  dfmp3.setRepeatPlayCurrentTrack(true);
}

void AlarmPlayer::stopAlarmPlayer() {
  if(Serial) Serial.println("Stopping alarm player");
  alarmEnabled = false;
  dfmp3.stop();
  dfmp3.setVolume(0);
}

void AlarmPlayer::manageAlarmPlayer() {
  if(alarmEnabled) {
    if(snoozed) {
      if(millis() - startTime > snoozeTime) {
        startAlarmPlayer();
      }
    } else
    if(millis() - startTime > alarmTimeout) {
      alarmEnabled = false;
      dfmp3.stop();
    } else 
    if(millis() - volIncrease > 1500 && volume < 18) {
      if(Serial) {
        Serial.print("Increasing volume from ");
        Serial.println(volume);
      }
      volIncrease = millis();
      volume++;
      dfmp3.setVolume(0.05*volume*volume);
    }
  }
}

void AlarmPlayer::snoozeAlarmPlayer() {
  snoozed = true;
  dfmp3.stop();
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
    dfmp3.setVolume(20);
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

// TODO: need two variables, one for alarm and one for regular music
// TODO: need to check for bounds
/*void playTrack(int track) {
  if(!alarmIsPlaying) {
    Serial.print("playing track: ");
    Serial.println(track);
    alarmIsPlaying = true;
    dfmp3.setVolume(10);
    dfmp3.playMp3FolderTrack(track);
    dfmp3.setRepeatPlayCurrentTrack(true);
  }
}*/

void playTrack(int track) {
  Serial.print("playing track: ");
  Serial.println(track);
  dfmp3.setVolume(10);
  dfmp3.playMp3FolderTrack(track);
  dfmp3.playMp3FolderTrack(track);
  dfmp3.setRepeatPlayCurrentTrack(true);
}

void stopTrack() {
  if(alarmIsPlaying) {
    dfmp3.stop();
    alarmIsPlaying = false;
  }
}

void manageDFPlayer() {
    dfmp3.loop();
}