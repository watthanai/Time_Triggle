#define LED_Relay 5
#ifdef ESP32
#include <WiFi.h>
#include <WiFiMulti.h>
#endif
#include <time.h>
extern "C" {
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
}
#include "src/LiquidCrystal_I2C.h"



////////////////////////
// Config Wifi//
////////////////////////
typedef struct {
  char ssid[64] = "EmOne_2.4G";
  char pass[64] = "nice2meetu";
} config_wf;
config_wf cfg_wifi;
TimerHandle_t wifiReconnectTimer;


////////////////////////
// Config Timezone//
///////////////////////
typedef struct {
  char NTP_SERVER[64] = "ch.pool.ntp.org";
  const long GMTOffset = 25200; // offset time 3600*7
  char time_output[30];
  char starttime[30];
  char endtime[30];
} config_t;
config_t cfg_TZ;
tm timeinfo;
time_t now;


////////////////////////
// Config Sensor//
///////////////////////
typedef struct {
  uint8_t Temp = 25;
  uint8_t Temp_deci = 0;
} sensor_t;
sensor_t sensor;
TimerHandle_t sensorTimer;



////////////////////////
// Config LCD//
///////////////////////rows
uint8_t lcdColumns = 16;
uint8_t lcdRows = 2;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);

/////////**************///////////////
// Setup
/////////*************//////////////


//******WIFI***********//
void wifi_setup() {
  Serial.println("WIFI: SETUP");
  wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(wifi_connect));
  wifi_connect();
}

void wifi_connect() {
  Serial.println("WIFI: Connecting...");
  WiFi.begin(cfg_wifi.ssid, cfg_wifi.pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);

    Serial.print(".");
  }
  Serial.println("WIFI: Connected");
  Serial.print("WIFI: IP address=");
  Serial.println(WiFi.localIP());
}
//*******************//


///*****Timezone*******//
TimerHandle_t TimezoneTimer;
void Timezone_setup() {
  //  Serial.println("Timezone: SETUP");
  configTime(cfg_TZ.GMTOffset, 0, cfg_TZ.NTP_SERVER);
  TimezoneTimer = xTimerCreate("TimezoneTimer", pdMS_TO_TICKS(1000), pdTRUE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(getNTPtime));
  xTimerStart(TimezoneTimer, 0);
}

void getNTPtime() {

  uint32_t start = millis();
  do {
    time(&now);
    localtime_r(&now, &timeinfo);
    delay(10);
  } while (((millis() - start) <= (1000 * 1)) && (timeinfo.tm_year < (2016 - 1900)));
  strftime(cfg_TZ.time_output, 30, "%d%m%y %H:%M:%S", localtime(&now));
  //  Serial.println(cfg_TZ.time_output);
}
//*******************//


///*****Sensor*******//
void sensor_setup() {

  sensorTimer = xTimerCreate("sensorTimer", pdMS_TO_TICKS(1000), pdTRUE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(sensor_update));
  xTimerStart(sensorTimer, 0);
}
void sensor_update() {
  sensor.Temp_deci = random(0, 99);
}
//*******************//



///*****LCD*******//
void LCD_display() {
  // set cursor to first column, first row
  String Temp = String(sensor.Temp) + "." + String(sensor.Temp_deci);
  lcd.setCursor(0, 0);
  // print message
  lcd.print(cfg_TZ.time_output);
  // set cursor to first column, second row
  lcd.setCursor(0, 1);
  lcd.print("Temp: ");
  lcd.print(Temp);     // display the temperature


}
//*******************//


///*****Triggle*******//
void Triggle(String Time)
{
  //  Serial.println(Time);
  String t = Time.substring(13, 15);


  uint16_t sec = t.toInt() ;
  uint16_t mod_sec = sec % 10;
  if (mod_sec == 0) {
    digitalWrite(LED_Relay, 1);
  }
  else {
    digitalWrite(LED_Relay, 0);

  }



}

//*******************//









void setup() {
  Serial.begin(115200);

  pinMode(LED_Relay, OUTPUT);
  wifi_setup();
  Timezone_setup();
  sensor_setup();
  lcd.init();
  lcd.backlight();
}

void loop() {
  LCD_display();
  Triggle(cfg_TZ.time_output);
}
