#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebOTA.h>
#include "TimeLib.h"
#include <ESP_EEPROM.h>

// Ukuran EEPROM (pastikan cukup untuk semua data)
#define EEPROM_SIZE 200

ESP8266WebServer server(80);

#define led1 D4
#define led2 D2
#define led3 D1

#define led_yellow D7 //kuning
#define led_green D0

IPAddress local_IP(192, 168, 4, 1);      // IP Address untuk AP
IPAddress gateway(192, 168, 4, 1);       // Gateway
IPAddress subnet(255, 255, 255, 0);      // Subnet mask

bool  stateAuto;
bool  stateLed1;
bool  stateLed2;
bool  stateLed3;
bool stateConnect;

char ssid[]     = "KELUARGA02";
char password[] = "bungasari";

#define BOARD_LED_BRIGHTNESS 255  // Kecerahan maksimum 244 dari 255
#define DIMM(x) ((uint32_t)(x) * (BOARD_LED_BRIGHTNESS) / 255)
uint8_t m_Counter = 0;   // Penghitung 8-bit untuk efek breathe
uint32_t beat[] = {100, 200};

void AP_init(){
  // Konfigurasi hotspot WiFi dari ESP8266
  WiFi.softAP(ssid);
  WiFi.softAPConfig(local_IP, gateway, subnet);
 
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  // Atur server untuk menerima permintaan set waktu, tanggal, teks, dan kecerahan
  server.on("/setLamp", handleSetTime);
  server.begin();
  Serial.println("Server dimulai.");  
}

// Fungsi untuk menghubungkan ke WiFi router
void WiFiConnect() {
  WiFi.mode(WIFI_STA); // Atur mode Station
  WiFi.begin(ssid, password); // Hubungkan ke WiFi router
  WiFi.softAPConfig(local_IP, gateway, subnet);

  Serial.print("Menghubungkan ke WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi terhubung!");
  Serial.print("Alamat IP: ");
  Serial.println(WiFi.localIP()); // Tampilkan alamat IP yang diberikan router
  // Atur server untuk menerima permintaan set waktu, tanggal, teks, dan kecerahan
  server.on("/setLamp", handleSetTime);
  server.begin();
  Serial.println("Server dimulai.");  
}


// Fungsi untuk mengatur jam, tanggal, running text, dan kecerahan
void handleSetTime(){
  Serial.println("hansle run");
  //static int flag = 0;
  //Buzzer(1);
  if (server.hasArg("WIFI")) {
    stateConnect =  server.arg("WIFI").toInt(); 
    server.send(200, "text/plain", (stateConnect)?"CONNECTED":"DISCONNECT");
  }
  if (server.hasArg("led1")) {
    stateLed1 = server.arg("led1").toInt(); 
    showLedClip(1);
    server.send(200, "text/plain", (stateLed1==1)?"led 1 ON" : "led 1 OFF");
  }
  if (server.hasArg("led2")) {
    stateLed2 = server.arg("led2").toInt(); 
    showLedClip(1);
    server.send(200, "text/plain", (stateLed2==1)?"led 2 ON" : "led 2 OFF");
  }
  if (server.hasArg("led3")) {
    stateLed3 = server.arg("led3").toInt(); 
    showLedClip(1);
    server.send(200, "text/plain", (stateLed3==1)?"led 3 ON" : "led 3 OFF");
  }
  if (server.hasArg("auto")) {
    stateAuto = server.arg("auto").toInt(); 
    showLedClip(1);
    server.send(200, "text/plain", (stateAuto==1)?"stateAuto ON" : "stateAuto OFF");
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
  } 
  delay(100);
  showLedClip(0);
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
  http://192.168.2.1:8080/webota
  pinMode(led_yellow, OUTPUT);
  pinMode(led_green, OUTPUT);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  //digitalWrite(led_pin, LOW);
  WiFiConnect();   //Inisialisasi Access Pointt
  
}

void loop() {
  // put your main code here, to run repeatedly:
 server.handleClient();
 webota.handle();
 
 if(stateLed1){  digitalWrite(led1, LOW); }
 else         {  digitalWrite(led1, HIGH); }

 if(stateLed2){  digitalWrite(led2, LOW); }
 else         {  digitalWrite(led2, HIGH); }

 if(stateLed3){  digitalWrite(led3, LOW); }
 else         {  digitalWrite(led3, HIGH); }

// if(stateLed1){  digitalWrite(led_pin, LOW); }
// else         {  digitalWrite(led_pin, LOW); }

 showLedIndi(stateConnect);
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
