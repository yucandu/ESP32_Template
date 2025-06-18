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

bool isSetNtp = false;

void cbSyncTime(struct timeval *tv) { // callback function to show when NTP was synchronized
  Serial.println("NTP time synched");
  isSetNtp = true;
}

void initTime(String timezone){
  configTzTime(timezone.c_str(), "192.168.50.197");
  while (!isSetNtp) {
        delay(250);
        }
}

WidgetTerminal terminal(V10);

#define every(interval) \
    static uint32_t __every__##interval = millis(); \
    if (millis() - __every__##interval >= interval && (__every__##interval = millis()))

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
  Serial.println("HTTP server started");
  delay(250);
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
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
  terminal.flush();
}

void loop() {
      ArduinoOTA.handle();
      if (WiFi.status() == WL_CONNECTED) {Blynk.run();}
      every(1000){
      Blynk.virtualWrite(V1, 69420);
      }

}
