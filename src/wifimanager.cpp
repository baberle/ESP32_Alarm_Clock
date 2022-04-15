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

bool WiFiManager::connectSpecific(const char* ssid, const char* password) {
    long startTime = millis();
    Serial.printf("Connecting to %s, password %s", ssid, password);
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

bool WiFiManager::connectSpecific(const char* ssid) {
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

bool WiFiManager::connectGeneral(fs::FS &fs) {

    bool success = false;
    bool knownNetwork = false;

    int n = WiFi.scanNetworks();
    if(n <= 0) {
        Serial.println("No networks to connect to!");
        return false;
    }

    if (!SPIFFS.begin(true)) {
        Serial.println("An error has occurred while mounting SPIFFS");
        return false;
    } else {
        Serial.println("SPIFFS mounted successfully");
    }

    Serial.printf("Reading file: /wifi.csv\n");
    File file = fs.open("/wifi.csv");
    if(!file || file.isDirectory()) {
        Serial.println("- failed to open file for reading");
        return false;
    }

    file.readStringUntil('\n'); // Discard first line
    while(file.available() && !success) {
        String ssid = file.readStringUntil(',');
        String pass = file.readStringUntil('\n');
        for(int i = 0; i < n && !success; i++) {
            if(WiFi.SSID(i) == ssid) {
                knownNetwork = true;
                success = connectSpecific(ssid.c_str(), pass.c_str());
                if(success) {
                    _ssid = ssid;
                    _password = pass;
                }
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
                success = connectSpecific(WiFi.SSID(i).c_str());
                if(success) {
                    _ssid = WiFi.SSID(i);
                    _password.clear();
                }
            }
        }
    }

    if(!success) {
        Serial.println("Could not connect to network!");
        return false;
    } else {
        return true;
    }
}

/*void WiFiManager::onDisconnect(WiFiEvent_t event, WiFiEventInfo_t info){
  Serial.println("Disconnected from WiFi access point");
  Serial.print("WiFi lost connection. Reason: ");
  Serial.println(info.disconnected.reason);
  Serial.println("Trying to Reconnect");
  _disconnectTime = millis();
  if(_password.length() != 0) connectSpecific(_ssid.c_str(), _password.c_str());
  else connectSpecific(_ssid.c_str());
}

void WiFiManager::onConnect(WiFiEvent_t event, WiFiEventInfo_t info){
  //Serial.println("Disconnected from WiFi access point");
  //Serial.print("WiFi lost connection. Reason: ");
  //Serial.println(info.disconnected.reason);
  _disconnectTime = 0;
}*/

void WiFiManager::setup() {

    WiFi.mode(WIFI_STA);
    WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
    WiFi.setHostname(_hostname.c_str());

    //WiFi.onEvent(onDisconnect, SYSTEM_EVENT_STA_DISCONNECTED);

    connectGeneral(SPIFFS);

    //WiFi.onEvent(WiFiStationConnected, SYSTEM_EVENT_STA_CONNECTED);
    //WiFi.onEvent(WiFiStationDisconnected, SYSTEM_EVENT_STA_DISCONNECTED);
}

void WiFiManager::addNetwork(fs::FS &fs, const char* ssid, const char* password) {
    // TODO: Do I need to do this in every one?
    // TODO: Should only add to network if know the password is connect
    if (!SPIFFS.begin(true)) {
        Serial.println("An error has occurred while mounting SPIFFS");
        return;
    } else {
        Serial.println("SPIFFS mounted successfully");
    }
    Serial.printf("Appending to file: /wifi.csv\n");
    File file = fs.open("/wifi.csv", FILE_APPEND);
    if(!file){
        Serial.println("- failed to open file for appending");
        return;
    }
    String line;
    line.concat(ssid);
    line.concat(',');
    line.concat(password);
    line.concat('\n');
    if(file.print(line)){
        Serial.println("- line appended");
    } else {
        Serial.println("- append failed");
    }
    file.close();
}

unsigned long WiFiManager::timeDisconnected() {
    if(WiFi.status() == WL_CONNECTED) return 0;
    else return millis() - _disconnectTime;
}




