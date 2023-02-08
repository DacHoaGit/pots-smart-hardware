
#include <Firebase_ESP_Client.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncTCP.h>
#include "LittleFS.h"
#include <ESPAsyncWebServer.h>
#include <Wire.h>

char buff[50];

unsigned long previousMillis = 0;
unsigned long currentMillis;
const unsigned long period = 10000;

AsyncWebServer server(80);
// Provide the token generation process info.

// Provide the RTDB payload printing info and other helper functions.
// #include <addons/RTDBHelper.h>

/* 1. Define the WiFi credentials */
#define WIFI_SSID "Wifii"
#define WIFI_PASSWORD "88888889"


#define API_KEY "AIzaSyDTwNIC2GwRtA_2NEBvPjkaYG_E6ciEzsU"
#define USER_EMAIL1 "ledachoa01@gmail.com"
#define USER_PASSWORD1 "01653922788Nd"
#define DATABASE_URL "https://pots-smart-default-rtdb.firebaseio.com/"  //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app

/* 5. Define the Firebase Data object */
FirebaseData fbdo;
// FirebaseConfig configFB;
FirebaseJson json;
// FirebaseJson updateData;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
int timestamp;
unsigned long sendDataPrevMillis = 0;
unsigned long timerDelay = 60000;

int count = 0;
int indx = 0;


FirebaseAuth auth1;
FirebaseConfig config1;

String ssid;
String pass;
String ip;
String gateway;
String token;

const char* ssidPath = "/ssid.txt";
const char* tokenPath = "/token.txt";
const char* passPath = "/pass.txt";
const char* ipPath = "/ip.txt";
const char* gatewayPath = "/gateway.txt";

int nhiet_do_den_min = 0;
int nhiet_do_den_max = 0;
int nhiet_do_bom_min = 0;
int nhiet_do_bom_max = 0;
int do_am_dat_den_min = 0;
int do_am_dat_den_max = 0;
int do_am_dat_bom_min = 0;
int do_am_dat_bom_max = 0;
int do_am_khong_khi_den_min = 0;
int do_am_khong_khi_den_max = 0;
int do_am_khong_khi_bom_min = 0;
int do_am_khong_khi_bom_max = 0;

IPAddress localIP;
IPAddress localGateway;
IPAddress subnet(255, 255, 0, 0);

// Initialize LittleFS
void initFS() {
  if (!LittleFS.begin()) {
    Serial.println("An error has occurred while mounting LittleFS");
  } else {
    Serial.println("LittleFS mounted successfully");
  }
}

String readFile(fs::FS& fs, const char* path) {
  Serial.printf("Reading file: %s\r\n", path);

  File file = fs.open(path, "r");
  if (!file || file.isDirectory()) {
    Serial.println("- failed to open file for reading");
    return String();
  }

  String fileContent;
  while (file.available()) {
    fileContent = file.readStringUntil('\n');
    break;
  }
  file.close();
  return fileContent;
}

// Write file to LittleFS
void writeFile(fs::FS& fs, const char* path, const char* message) {
  Serial.printf("Writing file: %s\r\n", path);

  File file = fs.open(path, "w");
  if (!file) {
    Serial.println("- failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("- file written");
  } else {
    Serial.println("- frite failed");
  }
  file.close();
}

bool initWiFi() {
  if (ssid == "" || ip == "" || token == "") {
    Serial.println("Undefined SSID or IP address.");
    return false;
  }

  // WiFi.mode(WIFI_STA);
  // localIP.fromString(ip.c_str());
  // localGateway.fromString(gateway.c_str());

  // if (!WiFi.config(localIP, localGateway, subnet)) {
  //   Serial.println("STA Failed to configure");
  //   return false;
  // }
  WiFi.begin(ssid, pass);

  Serial.println("Connecting to WiFi...");
  delay(20000);
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Failed to connect.");
    return false;
  }
  Serial.println(WiFi.localIP());
  return true;
}



unsigned long getTime() {
  timeClient.update();
  unsigned long now = timeClient.getEpochTime();
  return now;
}



int nhietdo = 0;
int doamdat = 0;
int doamkhongkhi = 0;
int status_den = 0;
int status_bom = 0;
int config = 2;
int status_auto = 0;

const char* PARAM_INPUT_1 = "ssid";
const char* PARAM_INPUT_2 = "pass";
const char* PARAM_INPUT_3 = "ip";
const char* PARAM_INPUT_4 = "gateway";
const char* PARAM_INPUT_5 = "token";

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>

<head>
  <title>POTS-SMART Wi-Fi Manager</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css"
    integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <style>
    html {
      font-family: Arial, Helvetica, sans-serif;
      display: inline-block;
      text-align: center;
    }

    h1 {
      font-size: 1.8rem;
      color: white;
    }

    p {
      font-size: 1.4rem;
    }

    .topnav {
      overflow: hidden;
      background-color: #0c713d;
    }

    body {
      margin: 0;
    }

    .content {
      padding: 5%;
    }

    .card-grid {
      max-width: 800px;
      margin: 0 auto;
      display: grid;
      grid-gap: 2rem;
      grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
    }

    .card {
      background-color: white;
      box-shadow: 2px 2px 12px 1px rgba(140, 140, 140, .5);
    }

    .card-title {
      font-size: 1.2rem;
      font-weight: bold;
      color: #034078
    }

    input[type=submit] {
      border: none;
      color: #FEFCFB;
      background-color: #0c713d;
      padding: 15px 15px;
      text-align: center;
      text-decoration: none;
      display: inline-block;
      font-size: 16px;
      width: 100px;
      margin-right: 10px;
      border-radius: 4px;
      transition-duration: 0.4s;
    }

    input[type=submit]:hover {
      background-color: #068443d6;
    }

    input[type=text],
    input[type=number],
    select {
      width: 50%;
      padding: 12px 20px;
      margin: 18px;
      display: inline-block;
      border: 1px solid #ccc;
      border-radius: 4px;
      box-sizing: border-box;
    }

    label {
      font-size: 1.2rem;
    }
    .row {
      width: 100%;
      display: flex;
      justify-content: center;
      align-items: center;
    }
    .row label {
      width: 30%;
      display: flex;
      justify-content: start;

    }
    .row input {
      display: flex;
      justify-content: end;
    }
    .value {
      font-size: 1.2rem;
      color: #1282A2;
    }

    .state {
      font-size: 1.2rem;
      color: #1282A2;
    }


    button {
      border: none;
      color: #FEFCFB;
      padding: 15px 32px;
      text-align: center;
      font-size: 16px;
      width: 100px;
      border-radius: 4px;
      transition-duration: 0.4s;
    }

    .button-on {
      background-color: #034078;
    }

    .button-on:hover {
      background-color: #1282A2;
    }

    .button-off {
      background-color: #858585;
    }

    .button-off:hover {
      background-color: #252524;
    }
  </style>
</head>

<body>
  <div class="topnav">
    <h1>POTS-SMART Wi-Fi Manager</h1>
  </div>
  <div class="content">
    <div class="card-grid">
      <div class="card">
        <form action="/" method="POST">
          <p>
          <div class="row">
            <label for="token">Token</label>
            <input type="text" id="token" name="token"><br>
          </div>
          <div class="row">
            <label for="ssid">Wifi Name</label>
            <input type="text" id="ssid" name="ssid"><br>
          </div>
          <div class="row">
            <label for="pass">Password</label>
            <input type="text" id="pass" name="pass"><br>
          </div>
          <div class="row">
            <label for="ip">IP Address</label>
            <input type="text" id="ip" name="ip" value="192.168.1.200"><br>
          </div>
          <div class="row">
            <label for="gateway">Gateway Address</label>
            <input type="text" id="gateway" name="gateway" value="192.168.1.1"><br>
          </div>
          <input type="submit" value="Submit">
          </p>
        </form>
      </div>
    </div>
  </div>
</body></html>)rawliteral";

boolean restart = false;

void changeWifi() {
  Serial.println("Setting AP (Access Point)");
  // NULL sets an open Access Point
  WiFi.softAP("ESP-WIFI-MANAGER", "88888888");

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  // Web Server Root URL
  // server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
  //   request->send(LittleFS, "./data/wifimanager.html", "text/html");
  // });
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send_P(200, "text/html", index_html);
  });
  server.serveStatic("/", LittleFS, "/");

  server.on("/", HTTP_POST, [](AsyncWebServerRequest* request) {
    int params = request->params();
    for (int i = 0; i < params; i++) {
      AsyncWebParameter* p = request->getParam(i);
      if (p->isPost()) {
        // HTTP POST ssid value
        if (p->name() == PARAM_INPUT_1) {
          ssid = p->value().c_str();
          Serial.print("SSID set to: ");
          Serial.println(ssid);
          // Write file to save value
          writeFile(LittleFS, ssidPath, ssid.c_str());
        }
        // HTTP POST pass value
        if (p->name() == PARAM_INPUT_2) {
          pass = p->value().c_str();
          Serial.print("Password set to: ");
          Serial.println(pass);
          // Write file to save value
          writeFile(LittleFS, passPath, pass.c_str());
        }
        // HTTP POST ip value
        if (p->name() == PARAM_INPUT_3) {
          ip = p->value().c_str();
          Serial.print("IP Address set to: ");
          Serial.println(ip);
          // Write file to save value
          writeFile(LittleFS, ipPath, ip.c_str());
        }
        // HTTP POST gateway value
        if (p->name() == PARAM_INPUT_4) {
          gateway = p->value().c_str();
          Serial.print("Gateway set to: ");
          Serial.println(gateway);
          // Write file to save value
          writeFile(LittleFS, gatewayPath, gateway.c_str());
        }
        if (p->name() == PARAM_INPUT_5) {
          token = p->value().c_str();
          Serial.print("Token set to: ");
          Serial.println(token);
          // Write file to save value
          writeFile(LittleFS, tokenPath, token.c_str());
        }
        initWiFi();
        //Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
      }
    }
    restart = true;
    request->send(200, "text/plain", "Done. ESP will restart, connect to your router and go to IP address: " + ip);
  });
  server.begin();
}

String data_arduino;


void serialFlush() {
  while (Serial.available() > 0) {
    char t = Serial.read();
  }
}
void setup() {

  Serial.begin(9600);



  initFS();
  // Load values saved in LittleFS
  ssid = readFile(LittleFS, ssidPath);
  pass = readFile(LittleFS, passPath);
  ip = readFile(LittleFS, ipPath);
  gateway = readFile(LittleFS, gatewayPath);
  token = readFile(LittleFS, tokenPath);

  if (initWiFi()) {
    Serial.println("Wifi connected ");
    Serial.print("[");

    delay(1000);
  } else {
    changeWifi();
  }

  timeClient.begin();
  config1.api_key = API_KEY;
  config1.database_url = DATABASE_URL;
  config1.signer.tokens.legacy_token = "jjLEC4C302QHAwcJy5fDqSwPAgBs5mi9k9TvlorJ";

  auth1.user.email = USER_EMAIL1;
  auth1.user.password = USER_PASSWORD1;

  Firebase.begin(&config1, &auth1);
  Serial.println(Firebase.ready());
}


void loop() {

  while (Serial.available() > 0) {
    data_arduino = Serial.readStringUntil('\n');
    Serial.println(data_arduino);
    if (data_arduino[0] != 'x')
      continue;
    char tem[50];
    char temp1[50];
    char temp2[50];
    char temp3[50];
    char temp4[50];
    char temp5[50];
    char temp6[50];
    char temp7[50];
    indx = 0;
    count = 0;
    for (int i = 0; i < data_arduino.length(); i++) {


      if (data_arduino[i] == 'x') {
        indx = 0;
        continue;
      }

      if (count == 0 && data_arduino[i] != 'a') {
        temp1[indx++] = data_arduino[i];
      }
      if (count == 0 && data_arduino[i] == 'a') {
        nhietdo = atof(temp1);
        count++;
        indx = 0;
        continue;
      }

      if (count == 1 && data_arduino[i] != 'b') {
        temp2[indx++] = data_arduino[i];
      }
      if (count == 1 && data_arduino[i] == 'b') {
        doamdat = atof(temp2);
        count++;
        indx = 0;
        continue;
      }

      if (count == 2 && data_arduino[i] != 'c') {
        temp3[indx++] = data_arduino[i];
      }
      if (count == 2 && data_arduino[i] == 'c') {
        doamkhongkhi = atof(temp3);
        count++;
        indx = 0;
        continue;
      }

      if (count == 3 && data_arduino[i] != 'd') {
        temp4[indx++] = data_arduino[i];
      }
      if (count == 3 && data_arduino[i] == 'd') {
        status_bom = atof(temp4);
        count++;
        indx = 0;
        continue;
      }

      if (count == 4 && data_arduino[i] != 'e') {
        temp5[indx++] = data_arduino[i];
      }
      if (count == 4 && data_arduino[i] == 'e') {
        status_den = atof(temp5);
        count++;
        indx = 0;
        continue;
      }

      if (count == 5 && data_arduino[i] != 'f') {
        temp6[indx++] = data_arduino[i];
      }
      if (count == 5 && data_arduino[i] == 'f') {
        config = atof(temp6);
        count++;
        indx = 0;
        continue;
      }

      if (count == 6 && data_arduino[i] != 'g') {
        temp7[indx++] = data_arduino[i];
      }
      if (count == 6 && data_arduino[i] == 'g') {
        status_auto = atof(temp7);
        count = 0;
        indx = 0;
        continue;
      }
    }
  }


  if (config == 0) {
    changeWifi();
    Serial.print("<");
    config = 2;
  }
  // Serial.end();
  // Serial.begin(9600);
  // break;
  // }

  if (Firebase.ready()) {

    if ((millis() - sendDataPrevMillis > timerDelay || sendDataPrevMillis == 0)) {
      sendDataPrevMillis = millis();
      FirebaseJson content;
      String documentPath = token + "/";
      content.set("fields/nhietdo/integerValue", String(nhietdo));
      content.set("fields/doamdat/integerValue", String(doamdat));
      content.set("fields/doamkhongkhi/integerValue", String(doamkhongkhi));
      content.set("fields/timestamp/integerValue", String(getTime()));
      Firebase.Firestore.patchDocument(&fbdo, "pots-smart", "" /* databaseId can be (default) or empty */, documentPath.c_str(), content.raw(), "nhietdo,doamdat,doamkhongkhi,timestamp" /* updateMask */);
      Firebase.Firestore.createDocument(&fbdo, "pots-smart", "", documentPath.c_str(), content.raw());
    }
    //Get current timestamp

    Firebase.RTDB.setInt(&fbdo, "/" + token + "/sensor/nhietdo", nhietdo);
    Firebase.RTDB.setInt(&fbdo, "/" + token + "/sensor/doamdat", doamdat);
    Firebase.RTDB.setInt(&fbdo, "/" + token + "/sensor/doamkhongkhi", doamkhongkhi);
    Firebase.RTDB.setInt(&fbdo, "/" + token + "/sensor/status_den", !status_den);
    Firebase.RTDB.setInt(&fbdo, "/" + token + "/sensor/status_bom", !status_bom);
    Firebase.RTDB.setInt(&fbdo, "/" + token + "/sensor/status_auto", status_auto);


    if (Firebase.RTDB.getInt(&fbdo, ("/" + token + "/sensor/bom")) && fbdo.intData() == 1) {
      Serial.print("+");
      Firebase.RTDB.setInt(&fbdo, ("/" + token + "/sensor/bom"), 0);
    }
    if (Firebase.RTDB.getInt(&fbdo, ("/" + token + "/sensor/bom")) && fbdo.intData() == 2) {
      Serial.print("(");
      Firebase.RTDB.setInt(&fbdo, ("/" + token + "/sensor/bom"), 0);
    }
    if (Firebase.RTDB.getInt(&fbdo, ("/" + token + "/sensor/den")) && fbdo.intData() == 1) {
      Serial.print("$");
      Firebase.RTDB.setInt(&fbdo, ("/" + token + "/sensor/den"), 0);
    }
    if (Firebase.RTDB.getInt(&fbdo, ("/" + token + "/sensor/den")) && fbdo.intData() == 2) {
      Serial.print("^");
      Firebase.RTDB.setInt(&fbdo, ("/" + token + "/sensor/den"), 0);
    }

    if (Firebase.RTDB.getInt(&fbdo, ("/" + token + "/sensor/auto")) && fbdo.intData() == 1) {
      Firebase.RTDB.getInt(&fbdo, ("/" + token + "/sensor/nhiet_do_den_min"));
      nhiet_do_den_min = fbdo.intData();
      Firebase.RTDB.getInt(&fbdo, ("/" + token + "/sensor/nhiet_do_den_max"));
      nhiet_do_den_max = fbdo.intData();
      Firebase.RTDB.getInt(&fbdo, ("/" + token + "/sensor/nhiet_do_bom_min"));
      nhiet_do_bom_min = fbdo.intData();
      Firebase.RTDB.getInt(&fbdo, ("/" + token + "/sensor/nhiet_do_bom_max"));
      nhiet_do_bom_max = fbdo.intData();
      Firebase.RTDB.getInt(&fbdo, ("/" + token + "/sensor/do_am_dat_den_min"));
      do_am_dat_den_min = fbdo.intData();
      Firebase.RTDB.getInt(&fbdo, ("/" + token + "/sensor/do_am_dat_bom_min"));
      do_am_dat_bom_min = fbdo.intData();
      Firebase.RTDB.getInt(&fbdo, ("/" + token + "/sensor/do_am_dat_den_max"));
      do_am_dat_den_max = fbdo.intData();
      Firebase.RTDB.getInt(&fbdo, ("/" + token + "/sensor/do_am_dat_bom_max"));
      do_am_dat_bom_max = fbdo.intData();
      Firebase.RTDB.getInt(&fbdo, ("/" + token + "/sensor/do_am_khong_khi_den_min"));
      do_am_khong_khi_den_min = fbdo.intData();
      Firebase.RTDB.getInt(&fbdo, ("/" + token + "/sensor/do_am_khong_khi_den_max"));
      do_am_khong_khi_den_max = fbdo.intData();
      Firebase.RTDB.getInt(&fbdo, ("/" + token + "/sensor/do_am_khong_khi_bom_min"));
      do_am_khong_khi_bom_min = fbdo.intData();
      Firebase.RTDB.getInt(&fbdo, ("/" + token + "/sensor/do_am_khong_khi_bom_max"));
      do_am_khong_khi_bom_max = fbdo.intData();
      Firebase.RTDB.setInt(&fbdo, ("/" + token + "/sensor/auto"), 0);

      Serial.print(")");
      Serial.print('x');
      Serial.print(nhiet_do_den_min);
      Serial.print('a');
      Serial.print(nhiet_do_den_max);
      Serial.print('b');
      Serial.print(nhiet_do_bom_min);
      Serial.print('c');
      Serial.print(nhiet_do_bom_max);
      Serial.print('d');
      Serial.print(do_am_dat_den_min);
      Serial.print('e');
      Serial.print((do_am_dat_bom_min));
      Serial.print('f');
      Serial.print((do_am_dat_den_max));
      Serial.print('g');
      Serial.print((do_am_dat_bom_max));
      Serial.print('h');
      Serial.print((do_am_khong_khi_den_min));
      Serial.print('i');
      Serial.print((do_am_khong_khi_den_max));
      Serial.print('k');
      Serial.print((do_am_khong_khi_bom_min));
      Serial.print('m');
      Serial.print((do_am_khong_khi_bom_max));
      Serial.println('n');
    }
    if (Firebase.RTDB.getInt(&fbdo, ("/" + token + "/sensor/auto")) && fbdo.intData() == 2) {
      Serial.print("@");
      Firebase.RTDB.setInt(&fbdo, ("/" + token + "/sensor/auto"), 0);
    }
  }
}
