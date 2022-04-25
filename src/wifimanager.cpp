#include "wifimanager.h"

static IPAddress ipAddr;
static bool inAPMode = false;
static String _ssid;
static String _password;
static unsigned long _disconnectTime;
static unsigned long _lastTimeReconnectAttempt;
static const String _hostname = "esp32device";


static bool connectSpecific(const char* ssid, const char* password) {
    long startTime = millis();
    Serial.printf("Connecting to %s, password %s", ssid, password);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        if(millis() - startTime > 10000) {
            Serial.println(" DISCONNECTED");
            return false;
        }
        delay(500);
        Serial.print(".");
    }
    Serial.println(" CONNECTED");
    return true;
}

static bool connectSpecific(const char* ssid) {
    long startTime = millis();
    Serial.printf("Connecting to %s ", ssid);
    WiFi.begin(ssid);
    while (WiFi.status() != WL_CONNECTED) {
        if(millis() - startTime > 10000) {
            Serial.println(" DISCONNECTED");
            return false;
        }
        delay(500);
        Serial.print(".");
    }
    Serial.println(" CONNECTED");
    return true;
}

static bool connectGeneral(fs::FS &fs) {

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
        if(!knownNetwork) deliverWebpage();
        return false;
    } else {
        return true;
    }
}


static void onDisconnect(WiFiEvent_t event, WiFiEventInfo_t info){
  Serial.println("Disconnected from WiFi access point");
  Serial.print("WiFi lost connection. Reason: ");
  Serial.println(info.disconnected.reason);
  Serial.println("Trying to Reconnect");
  _disconnectTime = millis();
  if(_password.length() != 0) connectSpecific(_ssid.c_str(), _password.c_str());
  else connectSpecific(_ssid.c_str());
}


static void onConnect(WiFiEvent_t event, WiFiEventInfo_t info){
  _disconnectTime = 0;
}


void setupWiFi() {

    WiFi.mode(WIFI_STA);
    WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
    WiFi.setHostname(_hostname.c_str());

    WiFi.onEvent(onDisconnect, SYSTEM_EVENT_STA_DISCONNECTED);
    WiFi.onEvent(onConnect, SYSTEM_EVENT_STA_CONNECTED);

    bool successfulConnection = connectGeneral(SPIFFS);

    if(!successfulConnection && !inApMode()) deliverWebpage();
}


void addNetwork(const char* ssid, const char* password) {

    if (!SPIFFS.begin(true)) {
        Serial.println("An error has occurred while mounting SPIFFS");
        return;
    } else {
        Serial.println("SPIFFS mounted successfully");
    }
    Serial.printf("Appending to file: /wifi.csv\n");
    File file = SPIFFS.open("/wifi.csv", FILE_APPEND);
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


void manageWiFi() {
    if(inApMode()) return;
    if(timeDisconnected() == 0) return;
    if(timeDisconnected() >= 60*60*1000) {
        // If Disconnected longer than 1 hour
        deliverWebpage();
    } else if(timeDisconnected() >= 10*60*1000 && _lastTimeReconnectAttempt - millis() >= 10*60*1000) {
        // If disconnected longer than 10 minutes
        // Every 10 minutes attempt reconnection
        _lastTimeReconnectAttempt = millis();
        connectGeneral(SPIFFS);
    }
}


unsigned long timeDisconnected() {
    if(WiFi.status() == WL_CONNECTED) return 0;
    else return (millis() - _disconnectTime);
}


bool inApMode() { return inAPMode; }


String getIpAddr() { return ipAddr.toString(); }


String getSelectOptions(const String& var) {

    String out;

    int n = WiFi.scanNetworks();
    if(n <= 0) {
        Serial.println("No networks to connect to!");
        return String();
    }

    for(int i = 0; i < n; i++) {
        String option = "<option value=\"" + WiFi.SSID(i) + "\">" + WiFi.SSID(i) + "</option>";
        out += option;
    }

    return out;
}


AsyncWebServer server(80);

static const char* PARAM_INPUT_1 = "ssid";
static const char* PARAM_INPUT_2 = "pass";

void deliverWebpage() {

    Serial.println("Setting AP (Access Point)");

    WiFi.softAP("Alarm_Clock", NULL);
    inAPMode = true;

    // TODO: This should be recorded globally
    IPAddress IP = WiFi.softAPIP();
    ipAddr = IP;
    Serial.print("AP IP address: ");
    Serial.println(IP); 

    // Web Server Root URL
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/wifimanager.html", "text/html", false, getSelectOptions);
    });
    
    server.serveStatic("/", SPIFFS, "/");
    
    server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
        int params = request->params();
        for(int i=0;i<params;i++){
            AsyncWebParameter* p = request->getParam(i);
            if(p->isPost()){
                String ssid = "";
                String pass = "";
                // HTTP POST ssid value
                if (p->name() == PARAM_INPUT_1) {
                    ssid = p->value();
                    Serial.print("SSID set to: ");
                    Serial.println(ssid);
                }
                // HTTP POST pass value
                if (p->name() == PARAM_INPUT_2) {
                    pass = p->value();
                    Serial.print("Password set to: ");
                    Serial.println(pass);
                }
                // Snaitize input (check network exists and stuff)
                if(pass.length() > 0) addNetwork(ssid.c_str(), pass.c_str());
            }
        }
        request->send(200, "text/plain", "Done. ESP will restart and try to connect");
        delay(2000);
        ESP.restart();
    });
    server.begin();
}