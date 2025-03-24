/*
 * 0     state connect
 * 1     auto
 * 2     led1
 * 3     led2
 * 4     led3
 * 5     jamOn
 * 13    meniton
 * 21    jamoff
 * 29    menitoff
 * 37
 */

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "TimeLib.h"
#include <ESP_EEPROM.h>
#include <NTPClient.h>

// Ukuran EEPROM (pastikan cukup untuk semua data)
#define EEPROM_SIZE 200

ESP8266WebServer server(80);

#define led1 D4
#define led2 D2
#define led3 D1

#define led_yellow D7 //kuning
#define led_green D0

IPAddress local_IP(192,168,100,8);      // IP Address untuk AP
IPAddress gateway(192,168,100,8);       // Gateway
IPAddress subnet(255, 255, 255, 0);      // Subnet mask

const long utcOffsetInSeconds = 25200;
WiFiUDP ntpUDP;
NTPClient Clock(ntpUDP, "asia.pool.ntp.org", utcOffsetInSeconds);

bool  stateAuto;
bool  stateLed1;
bool  stateLed2;
bool  stateLed3;
bool  stateConnect;

uint8_t jamOn,menitOn;
uint8_t jamOff,menitOff;

//uint8_t jam;
//uint8_t menit;
//uint8_t detik;

bool pointLed1 = true;
bool pointLed2 = true;
bool pointLed3 = true;

char ssidSTA[]     = "KELUARGA02";
char passwordSTA[] = "mawarmerah";

const char* host = "OTA-smartlamp";

char ssidAP[]      = "SMARTLAMP";

#define BOARD_LED_BRIGHTNESS 255  // Kecerahan maksimum 244 dari 255
#define DIMM(x) ((uint32_t)(x) * (BOARD_LED_BRIGHTNESS) / 255)
uint8_t m_Counter = 0;   // Penghitung 8-bit untuk efek breathe
uint32_t beat[] = {100, 200};

void wifiConnect() {
  WiFi.softAPdisconnect(true);
  WiFi.disconnect();
  delay(1000);

  if(stateConnect){
    Serial.println("Wifi Sation Mode");
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssidSTA, passwordSTA);
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED) {
      delay(50);
      Serial.print(".");
      beatLED();
      if (millis() - startTime > 10000) break;
    }
  
    if (WiFi.status() == WL_CONNECTED) {
      stateConnect = 1;
      Clock.begin();//NTP
      Clock.update();
      setTime(Clock.getHours(),Clock.getMinutes(),Clock.getSeconds(),12,02,2025);
    }else{
      Serial.println("Wifi AP Mode");
      WiFi.mode(WIFI_AP);
      WiFi.softAPConfig(local_IP, gateway, subnet);
      WiFi.softAP(ssidAP);
      stateConnect = 0;
     }
  }else{
    Serial.println("Wifi AP Mode");
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(local_IP, gateway, subnet);
    WiFi.softAP(ssidAP);
  }
  
  Serial.println("Server dimulai.");  
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

// Fungsi untuk mengatur jam, tanggal, running text, dan kecerahan
void handleSetTime(){
  Serial.println("hansle run");
  bool ok1 = false;
  
  if (server.hasArg("status")) { //auto,led1,led2,led3,wifi,
    server.send(200, "text/plain", "CONNECTED");
  }
  
  if (server.hasArg("WIFI")) {
    stateConnect =  server.arg("WIFI").toInt(); 
    wifiConnect();
    EEPROM.write(0,stateConnect);
    server.send(200, "text/plain", (stateConnect)?"CONNECTED":"DISCONNECT");
    ok1 = true;
  }
  
  if (server.hasArg("led1")) {
    stateLed1 = server.arg("led1").toInt(); 
    EEPROM.write(2,stateLed1);
    server.send(200, "text/plain", (stateLed1==1)?"led 1 ON" : "led 1 OFF");
    ok1 = true;
  }
  
  if (server.hasArg("led2")) {
    stateLed2 = server.arg("led2").toInt(); 
    EEPROM.write(3,stateLed2);
    server.send(200, "text/plain", (stateLed2==1)?"led 2 ON" : "led 2 OFF");
    ok1 = true;
  }
  
  if (server.hasArg("led3")) {
    stateLed3 = server.arg("led3").toInt(); 
    EEPROM.write(4,stateLed3);
    server.send(200, "text/plain", (stateLed3==1)?"led 3 ON" : "led 3 OFF");
    ok1 = true;
  }
  
  if (server.hasArg("auto")) {
    stateAuto = server.arg("auto").toInt(); 
    EEPROM.write(1,stateAuto);
    server.send(200, "text/plain", (stateAuto==1)?"stateAuto ON" : "stateAuto OFF");
    ok1 = true;
  }
  
  if (server.hasArg("setJam")) {
    String Jam = server.arg("setJam"); 
    int jam   = Jam.substring(0, 2).toInt();
    int menit = Jam.substring(3, 5).toInt();
    int detik = Jam.substring(6, 8).toInt();
    setTime(jam,menit,detik,25,12,24);
    char data[10];
    sprintf(data,"%02d:%02d:%02d",jam,menit,detik);
    Serial.print("setJam:");
    Serial.println(data);
    server.send(200, "text/plain", "jam diupdate");
  }
  
   if (server.hasArg("alarmOn")) {
    String alarmON = server.arg("alarmOn"); 
    int jam   = alarmON.substring(0, 2).toInt();
    int menit = alarmON.substring(3, 5).toInt();
    
    jamOn = jam;
    menitOn = menit;
    
    EEPROM.write(5,jam);
    EEPROM.write(13,menit);
    
    char data[10];
    sprintf(data,"ON=%02d:%02d",jamOn,menitOn);
    Serial.print("alarmOn:");
    Serial.println(data);
    server.send(200, "text/plain", data);
    ok1 = true;
   }
   
   if (server.hasArg("alarmOff")) {
    String alarmOFF = server.arg("alarmOff"); 
    int jam   = alarmOFF.substring(0, 2).toInt();
    int menit = alarmOFF.substring(3, 5).toInt();
  
    jamOff = jam;
    menitOff = menit;
    
    EEPROM.write(21,jam);
    EEPROM.write(29,menit);
    
    char data[10];
    sprintf(data,"OFF=%02d:%02d",jam,menit);
    Serial.print("alarmOff:");
    Serial.println(data);
    server.send(200, "text/plain", data);
    ok1 = true;
   }
   
   if (server.hasArg("jam")) {
    int jam = hour();
    int menit = minute();
    int detik = second();
    char data[10];
    sprintf(data,"jam:%02d:%02d",jam,menit);
    server.send(200, "text/plain", data);
    Serial.print("jam:");
    Serial.println(data);
   }
   
   if (server.hasArg("setPoint")) {
    String point = server.arg("setPoint"); 
    pointLed1 = point.substring(0, 1).toInt();
    pointLed2 = point.substring(1, 2).toInt();
    pointLed3 = point.substring(2, 3).toInt();

    Serial.println("pointLed1" + String(pointLed1));
    Serial.println("pointLed2" + String(pointLed2));
    Serial.println("pointLed3" + String(pointLed3));
    
    EEPROM.put(37,pointLed1);
    EEPROM.put(38,pointLed2);
    EEPROM.put(39,pointLed3);

//    char data[5];
//    sprintf(data,"%s:%s:%s",pointLed1,pointLed2,pointLed3);
    server.send(200, "text/plain", "set point done");
    ok1 = true;
   }
   
  /* if (server.hasArg("getData")) {
    char data[30];
    sprintf(data,"%s-%s-%s-%s-%s-%s-%s-%02d:%02d-%02d:%02d",
    stateAuto,stateLed1,stateLed2,stateLed3,pointLed1,pointLed2,pointLed3,jamOn,menitOn,jamOff,menitOff);
    server.send(200, "text/plain", data);
   }
  if (server.hasArg("newPassword")) {
    String newPassword = server.arg("newPassword");
    showLedClip(1);
    if(newPassword.length()==8){
      Serial.println(String()+"newPassword:"+newPassword);
      newPassword.toCharArray(password, newPassword.length() + 1); // Set password baru
      saveStringToEEPROM(56, password); // Simpan password AP
      server.send(200, "text/plain", "Password WiFi diupdate");
    }else{Serial.println("panjang password melebihi 8 karakter"); server.send(200, "text/plain", "panjang password melebihi 8 karakter");}
  } */

  // write the data to EEPROM
  if(ok1) EEPROM.commit();
  Serial.println((ok1) ? "First commit OK" : "Commit failed");
  
}

// Fungsi untuk menyimpan string ke EEPROM
void saveStringToEEPROM(int startAddr, const String &data) {
  int len = data.length();
  for (int i = 0; i < len; i++) {
    EEPROM.put(startAddr + i, data[i]);
  }
  EEPROM.put(startAddr + len, '\0'); // Null terminator
}

// Fungsi untuk membaca string dari EEPROM
String readStringFromEEPROM(int startAddr) {
  char data[100]; // Buffer untuk string yang akan dibaca
  int len = 0;
  unsigned char k;
  k = EEPROM.read(startAddr);
  while (k != '\0' && len < 100) { // Membaca hingga null terminator
    data[len] = k;
    len++;
    k = EEPROM.read(startAddr + len);
  }
  data[len] = '\0';
  return String(data);
}
    
void setup() {
  Serial.begin(115200);
  EEPROM.begin(EEPROM_SIZE); // Inisialisasi EEPROM dengan ukuran yang ditentukan

  loadEEPROM();
  pinMode(led_yellow, OUTPUT);
  pinMode(led_green, OUTPUT);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  digitalWrite(led1, HIGH);
  digitalWrite(led2, HIGH);
  digitalWrite(led3, HIGH);
  wifiConnect();   //Inisialisasi Access Pointt

  ArduinoOTA.setHostname(host);

  ArduinoOTA.onStart([]() {
    String type;
  if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
  } else {  // U_FS
      type = "filesystem";
  }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  /* setup the OTA server */
  ArduinoOTA.begin();
  server.on("/setLamp", handleSetTime);
  server.begin();
  
}

void loop() {
  // put your main code here, to run repeatedly:
 ArduinoOTA.handle();
 server.handleClient();
 //Disconnect();
 
 if(stateAuto){
    uint8_t jam   = hour();
    uint8_t menit = minute();
    uint8_t detik = second();
   showLedClip(1);
   Serial.print(jam);
   Serial.print(":");
   Serial.print(menit);
   Serial.print(":");
   Serial.print(detik);
   Serial.println();

   if(jam == jamOn && menit == menitOn && detik == 00){
     Serial.println("jamOn active");
     (pointLed1)?digitalWrite(led1, LOW):digitalWrite(led1, HIGH);
     delay(50);
     (pointLed2)?digitalWrite(led2, LOW):digitalWrite(led2, HIGH);
     delay(50);
     (pointLed3)?digitalWrite(led3, LOW):digitalWrite(led3, HIGH);
   }
   else if(jam == jamOff && menit == menitOff && detik == 00){
     Serial.println("jamOff active");
     (pointLed1)?digitalWrite(led1, HIGH):digitalWrite(led1, LOW);
     delay(50);
     (pointLed2)?digitalWrite(led2, HIGH):digitalWrite(led2, LOW);
     delay(50);
     (pointLed3)?digitalWrite(led3, HIGH):digitalWrite(led3, LOW);
   }
  }else{
     if(stateLed1){  digitalWrite(led1, LOW); }
     else         {  digitalWrite(led1, HIGH); }
    
     if(stateLed2){  digitalWrite(led2, LOW); }
     else         {  digitalWrite(led2, HIGH); }
    
     if(stateLed3){  digitalWrite(led3, LOW); }
     else         {  digitalWrite(led3, HIGH); }
     showLedClip(0);
   }
   showLedIndi(stateConnect);
}

//void Disconnect(){
//  if (WiFi.status() != WL_CONNECTED) {
//    delay(100);
//    stateConnect=0;
//    ESP.restart();
//  }
//}

void loadEEPROM(){
  stateConnect = EEPROM.read(0);
  stateAuto    = EEPROM.read(1);
  stateLed1    = EEPROM.read(2);
  stateLed2    = EEPROM.read(3);
  stateLed3    = EEPROM.read(4);
  jamOn        = EEPROM.read(5);
  menitOn      = EEPROM.read(13);
  jamOff       = EEPROM.read(21);
  menitOff     = EEPROM.read(29);
  pointLed1    = EEPROM.read(37);
  pointLed2    = EEPROM.read(38);
  pointLed3    = EEPROM.read(39);
  
  Serial.println("stateConnect:"+String(stateConnect));
  Serial.println("stateAuto   :"+String(stateAuto));
  Serial.println("stateLed1   :"+String(stateLed1));
  Serial.println("stateLed2   :"+String(stateLed2));
  Serial.println("stateLed3   :"+String(stateLed3));
  Serial.println("jamOn       :"+String(jamOn));
  Serial.println("menitOn     :"+String(menitOn));
  Serial.println("jamOff      :"+String(jamOff));
  Serial.println("menitOff    :"+String(menitOff));
  Serial.println("pointLed1   :"+String(pointLed1));
  Serial.println("pointLed2   :"+String(pointLed2));
  Serial.println("pointLed3   :"+String(pointLed3));
}

void showLedClip(uint8_t state){
  switch(state){
    case 0 : 
     digitalWrite(led_green,LOW);
    break;

    case 1 :
     digitalWrite(led_green,HIGH);
    break;
  };
}

void showLedIndi(uint8_t state){

  if(state==0){
    unsigned long currentMillis = millis();
    static unsigned long previousMillis;
    static unsigned long interval=5000;
    // Mengatur LED dengan waveLED jika interval waktu telah tercapai
    if (currentMillis - previousMillis >= interval / 256) {  // Menggunakan interval/256 sesuai logika waveLED
      previousMillis = currentMillis;  // Perbarui waktu sebelumnya
      interval = waveLED(0, interval);  // Panggil fungsi waveLED dan dapatkan interval berikutnya
      //Serial.println("interval:"+String(interval));
    }
  }
  else if(state == 1){
    unsigned long currentMillis = millis();
    static unsigned long previousMillis;
    static unsigned long interval=500;
    // Mengatur LED dengan waveLED jika interval waktu telah tercapai
    if (currentMillis - previousMillis >= interval / 256) {  // Menggunakan interval/256 sesuai logika waveLED
      previousMillis = currentMillis;  // Perbarui waktu sebelumnya
      interval = waveLED(0, interval);  // Panggil fungsi waveLED dan dapatkan interval berikutnya
      //Serial.println("interval:"+String(interval));
    }
  }
}

uint32_t waveLED(uint32_t, unsigned breathePeriod) {
    uint32_t brightness = (m_Counter < 128) ? m_Counter : 255 - m_Counter;

    setLED(DIMM(brightness * 2));  // Mengatur LED dengan kecerahan yang dihitung

    // Menggulung nilai m_Counter antara 0 hingga 255
    m_Counter = (m_Counter + 1) % 256;
    
    // Mengembalikan nilai interval (delay) untuk satu iterasi
    return breathePeriod;
}

void beatLED() {
  static unsigned long previousMillis;
  unsigned long currentMillis = millis();
  
  // Periksa apakah waktu untuk beat berikutnya telah tercapai
  if (currentMillis - previousMillis >= beat[m_Counter]) {
    previousMillis = currentMillis;  // Perbarui waktu sebelumnya

    // Mengatur LED untuk berkedip sesuai dengan pola beat
    if (m_Counter % 2 == 0) {
      setBeatLED(BOARD_LED_BRIGHTNESS);
    } else {
      setBeatLED(0);
    }

    // Lanjutkan ke beat berikutnya
    m_Counter = (m_Counter + 1) % (sizeof(beat) / sizeof(beat[0]));
  }
}

void setLED(uint8_t brightness) {
  analogWrite(led_yellow, brightness);
}

void setBeatLED(uint8_t brightness) {
  analogWrite(led_yellow, brightness);
}
