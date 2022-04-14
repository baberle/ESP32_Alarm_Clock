#include "wifimanager.h"

WiFiManager::WiFiManager() {
    _hostname = "esp32device";
    _ssid = "WirelessNW_2.4";
    _password = "red66dog";
}

WiFiManager::WiFiManager(const String hostname) {
    _hostname = hostname;
    _ssid = "WirelessNW_2.4";
    _password = "red66dog";
}

void WiFiManager::connect() {
  long startTime = millis();
  Serial.printf("Connecting to %s ", _ssid.c_str());
  WiFi.begin(_ssid.c_str(), _password.c_str());
  while (WiFi.status() != WL_CONNECTED) {
      if(millis() - startTime > 5000) {
        Serial.println(" DISCONNECTED");
        return;
      }
      delay(500);
      Serial.print(".");
  }
  Serial.println(" CONNECTED");
}

void WiFiManager::scan() {
  Serial.println("scan start");

  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0) {
      Serial.println("no networks found");
  } else {
      Serial.print(n);
      Serial.println(" networks found");
      for (int i = 0; i < n; ++i) {
          // Print SSID and RSSI for each network found
          Serial.print(i + 1);
          Serial.print(": ");
          Serial.print(WiFi.SSID(i));
          Serial.print(" (");
          Serial.print(WiFi.RSSI(i));
          Serial.print(")");
          Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
          delay(10);
      }
  }
  Serial.println("");
}

void WiFiManager::setup3() {
  WiFi.mode(WIFI_STA);
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.setHostname(_hostname.c_str());
  //WiFi.onEvent(WiFiStationConnected, SYSTEM_EVENT_STA_CONNECTED);
  //WiFi.onEvent(WiFiStationDisconnected, SYSTEM_EVENT_STA_DISCONNECTED);
  scan();
  connect();
}

bool WiFiManager::connect2(const char* ssid, const char* password) {
    long startTime = millis();
    Serial.printf("Connecting to %s ", ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        if(millis() - startTime > 5000) {
            Serial.println(" DISCONNECTED");
            return false;
        }
        delay(500);
        Serial.print(".");
    }
    Serial.println(" CONNECTED");
    return true;
}

bool WiFiManager::connect2(const char* ssid) {
    long startTime = millis();
    Serial.printf("Connecting to %s ", ssid);
    WiFi.begin(ssid);
    while (WiFi.status() != WL_CONNECTED) {
        if(millis() - startTime > 5000) {
            Serial.println(" DISCONNECTED");
            return false;
        }
        delay(500);
        Serial.print(".");
    }
    Serial.println(" CONNECTED");
    return true;
}

void WiFiManager::setup2() {

    WiFi.mode(WIFI_STA);
    WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
    WiFi.setHostname(_hostname.c_str());

    int n = WiFi.scanNetworks();
    if(n <= 0) Serial.println("No networks to connect to!");

    bool success = false;
    bool knownNetwork = false;

    for(int i = 0; i < n && !success; i++) {
        if(WiFi.SSID(i) == _ssid) {
            knownNetwork = true;
            success = connect2(_ssid.c_str(), _password.c_str());
        }
    }

    if(!knownNetwork) Serial.println("No known networks");
    else if(!success) Serial.println("Cannot connect to known networks!");

    // If could not connect to saved network, check for open networks
    if(WiFi.status() != WL_CONNECTED) {
        for(int i = 0; i < n && !success; i++) {
            if(WiFi.encryptionType(i) == WIFI_AUTH_OPEN) {
                success = connect2(WiFi.SSID(i).c_str());
            }
        }
    }

    if(!success) Serial.println("Could not connect to network!");
}

void WiFiManager::onDisconnect(WiFiEvent_t event, WiFiEventInfo_t info){
  Serial.println("Disconnected from WiFi access point");
  Serial.print("WiFi lost connection. Reason: ");
  Serial.println(info.disconnected.reason);
  //Serial.println("Trying to Reconnect");
  //WiFi.begin(_ssid, _password);
}

void WiFiManager::setup(fs::FS &fs) {

    bool success = false;
    bool knownNetwork = false;

    WiFi.mode(WIFI_STA);
    WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
    WiFi.setHostname(_hostname.c_str());

    int n = WiFi.scanNetworks();
    if(n <= 0) {
        Serial.println("No networks to connect to!");
        return;
    }

    if (!SPIFFS.begin(true)) {
        Serial.println("An error has occurred while mounting SPIFFS");
    } else {
        Serial.println("SPIFFS mounted successfully");
    }

    Serial.printf("Reading file: /wifi.csv\n");
    File file = fs.open("/wifi.csv");
    if(!file || file.isDirectory()) {
        Serial.println("- failed to open file for reading");
        return;
    }

    file.readStringUntil('\n'); // Discard first line
    while(file.available() && !success) {
        String ssid = file.readStringUntil(',');
        String pass = file.readStringUntil('\n');
        for(int i = 0; i < n && !success; i++) {
            if(WiFi.SSID(i) == ssid) {
                knownNetwork = true;
                success = connect2(ssid.c_str(), pass.c_str());
            }
        }
    }
    file.close();

    if(!knownNetwork) Serial.println("No known networks");
    else if(!success) Serial.println("Cannot connect to known networks!");

    // If could not connect to saved network, check for open networks
    if(WiFi.status() != WL_CONNECTED) {
        for(int i = 0; i < n && !success; i++) {
            if(WiFi.encryptionType(i) == WIFI_AUTH_OPEN) {
                success = connect2(WiFi.SSID(i).c_str());
            }
        }
    }

    if(!success) Serial.println("Could not connect to network!");
}

/* ==================== CSV CLASS ========================== */

CSV::CSV(const char* path) {
    _path = path;
}

void CSV::initSPIFFS() {
  if (!SPIFFS.begin(true)) {
    Serial.println("An error has occurred while mounting SPIFFS");
  }
  Serial.println("SPIFFS mounted successfully");
}

void CSV::print(fs::FS &fs) {
    Serial.printf("Reading file: %s\r\n", _path);
    File file = fs.open(_path);
    if(!file || file.isDirectory()) {
        Serial.println("- failed to open file for reading");
        return;
    }
    Serial.println("");
    while(file.available()) {
        Serial.write(file.read());
    }
    file.close();
    Serial.println("");
}

void CSV::addLine(fs::FS &fs, const String& line) {
    Serial.printf("Appending to file: %s\r\n", _path);
    File file = fs.open(_path, FILE_APPEND);
    if(!file){
        Serial.println("- failed to open file for appending");
        return;
    }
    if(file.print(line)){
        Serial.println("- line appended");
    } else {
        Serial.println("- append failed");
    }
    if(line.charAt(line.length()-1) != '\n') {
        file.print("\n");
    }
}

