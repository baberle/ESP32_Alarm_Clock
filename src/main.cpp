#include <Arduino.h>
#include <DFMiniMp3.h>

const int SPKR_RX = 9;
const int SPKR_TX = 10;

class Mp3Notify; 
typedef DFMiniMp3<HardwareSerial, Mp3Notify> DfMp3; 

DfMp3 dfmp3(Serial1);

// Each soundbyte should be 1 minute long and ramp from 0 to 15 volume in 10 seconds
const char *fileTitles[3] = {
  "Star Wars",
  "Indiana Jones",
  "Tintin"
};

/*bool alarmEnabled = true;
bool alarmStarted = false;
unsigned long startTime = 0;
unsigned long volIncrease = 0;
int alarmTrack;
void startAlarm(int track);*/

struct AlarmPlayer {
  /*DfMp3& player;*/
  int track;
  bool alarmEnabled;
  unsigned long startTime;
  unsigned long volIncrease;
  AlarmPlayer(const int _track/*, DfMp3& _dfmp3*/);
  void startAlarmPlayer();
  void stopAlarmPlayer();
  void snoozeAlarmPlayer();
  void manageAlarmPlayer();
};

AlarmPlayer::AlarmPlayer(const int _track/*, DfMp3& _dfmp3*/) {
  track = _track;
  /*player = _dfmp3;*/
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
    if(millis() - startTime > 60000) {
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


AlarmPlayer ap(1/*, dfmp3*/);

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

void setup() {
  Serial.begin(115200);
  Serial.println("initializing...");
  dfmp3.begin();
  uint16_t volume = dfmp3.getVolume();
  Serial.print("volume ");
  Serial.println(volume);
  dfmp3.setVolume(10);
  uint16_t count = dfmp3.getTotalTrackCount(DfMp3_PlaySource_Sd);
  Serial.print("files ");
  Serial.println(count);
  Serial.println("starting...");
  
  ap.startAlarmPlayer();
}

/*void waitMilliseconds(uint16_t msWait)
{
  uint32_t start = millis();
  
  while ((millis() - start) < msWait)
  {
    // if you have loops with delays, its important to 
    // call dfmp3.loop() periodically so it allows for notifications 
    // to be handled without interrupts
    dfmp3.loop(); 
    delay(1);
  }
}*/



/*void playAlarm(int track) {
  if(!alarmStarted) {
    startTime = millis();
    alarmStarted = true;
    dfmp3.playMp3FolderTrack(track);
    dfmp3.setRepeatPlayCurrentTrack(true);
    dfmp3.setVolume(0);
  } else 
  if(millis() - startTime > 60000) {
    alarmEnabled = false;
    alarmStarted = false;
    dfmp3.stop();
  } else
  if(dfmp3.getVolume() < 15 && millis() - volIncrease > 200) {
    dfmp3.increaseVolume();
    volIncrease = millis();
    Serial.print("Volume: ");
    uint16_t volume = dfmp3.getVolume();
    Serial.println(volume);
  }
}*/

/*void startAlarm(int track) {
  alarmTrack = track;
  alarmEnabled = true;
  playAlarm(track);
}*/

void alarmManager() {
  //if(alarmEnabled) playAlarm(alarmTrack);
  ap.manageAlarmPlayer();
  dfmp3.loop(); 
}

/*void stopAlarm() {
  alarmEnabled = false;
  alarmStarted = false;
  dfmp3.stop();
}*/

/*void snoozeAlarm() {

}*/

void loop() 
{
  alarmManager();
  /*Serial.println("track 1"); 
  dfmp3.playMp3FolderTrack(1);  // sd:/mp3/0001.mp3
  
  waitMilliseconds(10000);
  
  Serial.println("track 2"); 
  dfmp3.playMp3FolderTrack(2); // sd:/mp3/0002.mp3
  
  waitMilliseconds(10000);
  
  Serial.println("track 3");
  dfmp3.playMp3FolderTrack(3); // sd:/mp3/0002.mp3
  
  waitMilliseconds(10000); */
}