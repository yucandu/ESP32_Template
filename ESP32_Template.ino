#include <Arduino.h>
#include <ArduinoOTA.h>
#include <BlynkSimpleEsp32.h>
#include <WiFi.h>
#include "time.h"
#include "esp_sntp.h"

const char* ssid = "mikesnet";
const char* password = "springchicken";

int hours, mins, secs;

char auth[] = "xxxxxxxxxxxxxxx";
struct tm timeinfo;
bool isSetNtp = false;

#define every(interval) \
    static uint32_t __every__##interval = millis(); \
    if (millis() - __every__##interval >= interval && (__every__##interval = millis()))

void cbSyncTime(struct timeval *tv) { // callback function to show when NTP was synchronized
  Serial.println("NTP time synched");
  Serial.println("getlocaltime");
  getLocalTime(&timeinfo);

  time_t rawtime;
  struct tm* timeinfo;
  time(&rawtime);
  timeinfo = localtime(&rawtime);

  Serial.println(asctime(timeinfo));
  time_t now = time(nullptr); // local-adjusted time
  localTimeUnix = static_cast<uint32_t>(now); // 32-bit to send via ESP-NOW
  isSetNtp = true;
}


void initSNTP() {  
  sntp_set_sync_interval(10 * 60 * 1000UL);  // 1 hour
  sntp_set_time_sync_notification_cb(cbSyncTime);
  esp_sntp_setoperatingmode(ESP_SNTP_OPMODE_POLL);
  esp_sntp_setservername(0, "192.168.50.197");
  esp_sntp_init();
  wait4SNTP();
  setTimezone();
}

void wait4SNTP() {
  Serial.print("Waiting for time...");
  while (sntp_get_sync_status() != SNTP_SYNC_STATUS_COMPLETED) {
    delay(1000);
    Serial.print(".");
  }
}

void setTimezone() {  
  setenv("TZ","EST5EDT,M3.2.0,M11.1.0",1);
  tzset();
}

WidgetTerminal terminal(V10);

BLYNK_WRITE(V10) {
  if (String("help") == param.asStr()) {
    terminal.println("==List of available commands:==");
    terminal.println("wifi");
    terminal.println("==End of list.==");
  }
  if (String("wifi") == param.asStr()) {
    terminal.print("Connected to: ");
    terminal.println(ssid);
    terminal.print("IP address:");
    terminal.println(WiFi.localIP());
    terminal.print("Signal strength: ");
    terminal.println(WiFi.RSSI());
    printLocalTime();
  }
    terminal.flush();
}

void printLocalTime() {
  time_t rawtime;
  struct tm* timeinfo;
  time(&rawtime);
  timeinfo = localtime(&rawtime);
  terminal.print(asctime(timeinfo));
}

void setup(void) {
  sntp_set_time_sync_notification_cb(cbSyncTime);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
    WiFi.setTxPower(WIFI_POWER_8_5dBm); //low power for better connectivity
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

    
  ArduinoOTA.setHostname("myesp32");
    
    
  ArduinoOTA.begin();
  Serial.println("OTA started");
  delay(250);
  Serial.println("Init time");
  initSNTP();
  Serial.println("getlocaltime");
  getLocalTime(&timeinfo);

  time_t rawtime;
  struct tm* timeinfo;
  time(&rawtime);
  timeinfo = localtime(&rawtime);

  Serial.println(asctime(timeinfo));
  Blynk.config(auth, IPAddress(192, 168, 50, 197), 8080);
  Blynk.connect();
  delay(250);
  struct tm timeinfo;
  getLocalTime(&timeinfo);
  hours = timeinfo.tm_hour;
  mins = timeinfo.tm_min;
  secs = timeinfo.tm_sec;
  terminal.println("***SERVER STARTED***");
  terminal.print("Connected to ");
  terminal.println(ssid);
  terminal.print("IP address: ");
  terminal.println(WiFi.localIP());
  printLocalTime();
  terminal.print("Compiled on: ");
  terminal.print(__DATE__);
  terminal.print(" at ");
  terminal.println(__TIME__);
  terminal.flush();
}

void loop() {
      ArduinoOTA.handle();
      if (WiFi.status() == WL_CONNECTED) {Blynk.run();}
      every(1000){
      Blynk.virtualWrite(V1, 69420);
      }

}
