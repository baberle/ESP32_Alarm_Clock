#include "wifimanager.h"

/*WiFiManager::WiFiManager() {
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
}*/

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

/*void WiFiManager::setup() {

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
}*/




/* ========================= C-STYLE IMPLEMENTATION ============================ */


static String _ssid;
static String _password;
static unsigned long _disconnectTime;
static const String _hostname = "esp32device";


static bool connectSpecific(const char* ssid, const char* password) {
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

static bool connectSpecific(const char* ssid) {
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
  //Serial.println("Disconnected from WiFi access point");
  //Serial.print("WiFi lost connection. Reason: ");
  //Serial.println(info.disconnected.reason);
  _disconnectTime = 0;
}

void setupWiFi() {

    /*WiFi.mode(WIFI_STA);
    WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
    WiFi.setHostname(_hostname.c_str());

    WiFi.onEvent(onDisconnect, SYSTEM_EVENT_STA_DISCONNECTED);
    WiFi.onEvent(onConnect, SYSTEM_EVENT_STA_CONNECTED);

    connectGeneral(SPIFFS);*/

    deliverWebpage();
}

void addNetwork(fs::FS &fs, const char* ssid, const char* password) {
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

unsigned long timeDisconnected() {
    if(WiFi.status() == WL_CONNECTED) return 0;
    else return millis() - _disconnectTime;
}







// Initialize SPIFFS
void initSPIFFS() {
  if (!SPIFFS.begin(true)) {
    Serial.println("An error has occurred while mounting SPIFFS");
  }
  Serial.println("SPIFFS mounted successfully");
}

// Read File from SPIFFS
String readFile(fs::FS &fs, const char * path){
  Serial.printf("Reading file: %s\r\n", path);

  File file = fs.open(path);
  if(!file || file.isDirectory()){
    Serial.println("- failed to open file for reading");
    return String();
  }
  
  String fileContent;
  while(file.available()){
    fileContent = file.readStringUntil('\n');
    break;     
  }
  return fileContent;
}

// Write file to SPIFFS
void writeFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Writing file: %s\r\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file){
    Serial.println("- failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("- file written");
  } else {
    Serial.println("- frite failed");
  }
}

// Initialize WiFi
/*bool initWiFi() {
  if(ssid.equals("") || ip.equals("")){
    Serial.println("Undefined SSID or IP address.");
    return false;
  }

  WiFi.mode(WIFI_STA);
  localIP.fromString(ip.c_str());
  localGateway.fromString(gateway.c_str());


  if (!WiFi.config(localIP, localGateway, subnet)){
    Serial.println("STA Failed to configure");
    return false;
  }
  WiFi.begin(ssid.c_str(), pass.c_str());
  Serial.println("Connecting to WiFi...");

  unsigned long currentMillis = millis();
  previousMillis = currentMillis;

  while(WiFi.status() != WL_CONNECTED) {
    currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      Serial.println("Failed to connect.");
      return false;
    }
  }

  Serial.println(WiFi.localIP());
  return true;
}*/

AsyncWebServer server(80);

static const char* PARAM_INPUT_1 = "ssid";
static const char* PARAM_INPUT_2 = "pass";
static const char* PARAM_INPUT_3 = "ip";
static const char* PARAM_INPUT_4 = "gateway";

IPAddress localIP;
IPAddress localGateway;
IPAddress subnet(255, 255, 0, 0);

void deliverWebpage() {

    Serial.println("Setting AP (Access Point)");

    WiFi.softAP("ESP-WIFI-MANAGER", NULL);

    // TODO: This should be recorded globally
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP); 

    // Web Server Root URL
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/wifimanager.html", "text/html");
    });
    
    server.serveStatic("/", SPIFFS, "/");
    
    server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
        int params = request->params();
        for(int i=0;i<params;i++){
            AsyncWebParameter* p = request->getParam(i);
            if(p->isPost()){
                const char* ssid;
                const char* pass;
                // HTTP POST ssid value
                if (p->name() == PARAM_INPUT_1) {
                    ssid = p->value().c_str();
                    Serial.print("SSID set to: ");
                    Serial.println(ssid);
                    // Write file to save value
                    //writeFile(SPIFFS, ssidPath, ssid.c_str());
                }
                // HTTP POST pass value
                if (p->name() == PARAM_INPUT_2) {
                    pass = p->value().c_str();
                    Serial.print("Password set to: ");
                    Serial.println(pass);
                    // Write file to save value
                    //writeFile(SPIFFS, passPath, pass.c_str());
                }

                // Snaitize input (check network exists and stuff)
                addNetwork(SPIFFS, ssid, pass);

                // HTTP POST ip value
                if (p->name() == PARAM_INPUT_3) {
                    const char* ip = p->value().c_str();
                    Serial.print("IP Address set to: ");
                    Serial.println(ip);
                    // Write file to save value
                    //writeFile(SPIFFS, ipPath, ip.c_str());
                }
                // HTTP POST gateway value
                if (p->name() == PARAM_INPUT_4) {
                    const char* gateway = p->value().c_str();
                    Serial.print("Gateway set to: ");
                    Serial.println(gateway);
                    // Write file to save value
                    //writeFile(SPIFFS, gatewayPath, gateway.c_str());
                }
                //Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
            }
        }
        request->send(200, "text/plain", "Done. ESP will restart and try to connect");
        delay(2000);
        ESP.restart();
    });
    server.begin();
}