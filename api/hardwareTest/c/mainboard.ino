#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include "driver/i2s.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include "time.h"
#include <SPI.h>
#include <LoRa.h>

//wifi,server and client device creds
const char* ssid  = "";
const char* password = "";
const char* accessUrl = "https://anabi-smartsystems.com/api/token";
const char* dataUrl = "https://anabi-smartsystems.com/api/data";
const char* clientId = "1Jfkgsido75tN5WIPt70dREFdWOxSFhu";
const char* clientSecret = "eOtaVX1elukj1TPrCfgIYeImxdAdeCa4Bn1PlSoGMYOiAQPuIQEIFgkAmVcWgnDl";
const char* ntpServer = "pool.ntp.org";
//i2s
const i2s_port_t I2S_PORT = I2S_NUM_0; //i2s peripheral

//ds18b20
const int oneWireBus = 21;
//time
const long gmtoffsetSec = 3;
const int daylightOffsetSec = 10800;//offset in seconds for daylight saving time=> 1hr
//LoRa
#define ss 5
#define rst 14
#define dio0 2

OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);

void setup() {
  Serial.println("Anabi Main board initializing");
  Serial.begin(115200);
  //wifi config
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  //i2s config
  esp_err_t err;
  const i2s_config_t i2s_config = {
    .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX), // Receive, not transfer
    .sample_rate = 16000,                         // 16KHz
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT, // 24 from the datasheet doesnt fucking work 32
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT, // use right channel
    .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,     // Interrupt level 1
    .dma_buf_count = 4,                           // number of buffers
    .dma_buf_len = 8                              // 8 samples per buffer (minimum)
  };

  // The pin config ->configure the physical GPIO pins to which signals will be routed.
  const i2s_pin_config_t pin_config = {
    .bck_io_num = 26,   // Serial Clock (SCK)
    .ws_io_num = 33,    // Word Select (WS)
    .data_out_num = I2S_PIN_NO_CHANGE, // not used (only for speakers)
    .data_in_num = 32   // Serial Data (SD)
  };

  pinMode(32, INPUT);
  pinMode(26, OUTPUT);
  pinMode(33, OUTPUT);
  // Configuring the I2S driver and pins.
  // This function must be called before any I2S driver read/write operations.
  err = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  if (err != ESP_OK) {
    Serial.printf("Failed installing driver: %d\n", err);
    while (true);
  }
  err = i2s_set_pin(I2S_PORT, &pin_config);
  if (err != ESP_OK) {
    Serial.printf("Failed setting pin: %d\n", err);
    while (true);
  }
  Serial.println("I2S driver installed.");
  //lora init
  LoRa.setPins(ss, rst, dio0);
  Serial.println("LoRa Receiver");
  if (!LoRa.begin(915E6)) {
    Serial.println("Starting LoRa failed");
    while (1);
  }

}

void loop() {
  //float curnTemp = getSensorTemperature();
  //int32_t micVal = getMicData();
  //sendData(curnTemp,micVal);
  //getNtcTime();
  //delay(1000);
  receiveLoRa();
}

int32_t getMicData() {
  // Read a single sample and log it for the Serial Plotter.
  int32_t sample = 0;
  int bytes_read = i2s_pop_sample(I2S_PORT, (char *)&sample, portMAX_DELAY); // no timeout
  if (bytes_read > 0) {
    return sample;
  }
}
//clean the access token
String cleanAt(String at) {
  int fin = at.length() - 1;
  String out = at.substring(1, fin);
  return out;
}

float getSensorTemperature() {
  sensors.requestTemperatures();
  float temperatureC = sensors.getTempCByIndex(0);
  return temperatureC;
}

void getNtcTime() {
  struct tm timeinfo;
  configTime(gmtoffsetSec, daylightOffsetSec, ntpServer);
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain the time");
    return;
  }

  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  Serial.print("Day of week: ");
  Serial.println(&timeinfo, "%A");
  Serial.print("Month: ");
  Serial.println(&timeinfo, "%B");
  Serial.print("Day of Month: ");
  Serial.println(&timeinfo, "%d");
  Serial.print("Year: ");
  Serial.println(&timeinfo, "%Y");
  Serial.print("Hour: ");
  Serial.println(&timeinfo, "%H");
  Serial.print("Hour (12 hour format): ");
  Serial.println(&timeinfo, "%I");
  Serial.print("Minute: ");
  Serial.println(&timeinfo, "%M");
  Serial.print("Second: ");
  Serial.println(&timeinfo, "%S");
  Serial.println("Time variables");
  char timeHour[3];
  strftime(timeHour, 3, "%H", &timeinfo);
  Serial.println(timeHour);
  char timeWeekDay[10];
  strftime(timeWeekDay, 10, "%A", &timeinfo);
  Serial.println(timeWeekDay);
  Serial.println();

  /*
    //2021-05-10T23:00:03Z
    String cyear = (&timeinfo,"%Y");
    const char* cmonth = (&timeinfo,"%B");
    const char* cdate = (&timeinfo, "%d");
    const char* chour = (&timeinfo, "%H");
    const char* cmin = (&timeinfo, "%M");
    const char* csec = (&timeinfo, "%S");
    //format as iso 8601
    //String isoTime= String(cyear)+"-"+String(cmonth)+"-"+String(cdate)+"T"+String(chour)+":"+String(cmin)+":"+String(csec)+"Z";
    Serial.println(String(cmin));
  */
}

void sendData(float temp, int32_t micVal) {
  if (WiFi.status() == WL_CONNECTED) {
    //get the access token
    HTTPClient http;
    http.begin(accessUrl);
    http.addHeader("Content-Type", "application/json");
    //data
    //int httpResponseCode = http.POST("{\"client_id\":\"EptrWV0x17H0AlOoTvz06akdZ5imV4U5\",\"client_secret\":\"M5c70lGDh8g9Zp2cItFbQvIJnpGtJqX7tTwq3MC7wqWkR_9OD4j7iilAW0vVgzo9\",\"audience\":\"sams-dwh-web-api\",\"grant_type\":\"client_credentials\"}");
    int httpResponseCode = http.POST("{\"client_id\":\"" + String(clientId) + "\",\"client_secret\":\"" + String(clientSecret) + "\",\"audience\":\"sams-dwh-web-api\",\"grant_type\":\"client_credentials\"}");

    //Serial.print("HTTP Response code: ");
    //Serial.println(httpResponseCode);
    if (httpResponseCode == 200) {
      String payload = http.getString();
      JSONVar myobject = JSON.parse(payload);
      JSONVar accessToken = myobject["access_token"];
      String at = JSON.stringify(accessToken);
      //Serial.print("The access token: ");
      //Serial.println(accessToken);
      //Serial.println(at);
      http.end();

      //post data to anabi server
      HTTPClient http;
      http.begin(dataUrl);
      http.addHeader("Content-Type", "application/json");
      String authHeader = "Bearer " + cleanAt(at) + "";
      Serial.println(authHeader);
      http.addHeader("Authorization", "Bearer " + cleanAt(at) + "");
      //int httpResponseCode = http.POST("{\"sourceId\":\"temp\",\"values\": \"{\"ts\":\"2021-05-07T16:03:03Z\",\"value\":\"" + String(temp) + "\"}\"}");
      //int httpResponseCode = http.POST("{\"sourceId\":\"temp\",\"values\": \"[\"{\"ts\":\"2021-05-07T16:03:03Z\",\"value\":\"" + String(temp) + "\"\"}\"\"]\"}");
      //int httpResponseCode = http.POST("{\"sourceId\":\"temp\",\"values\":[{\"ts\":\"2021-05-08T11:03:03Z\",\"value\":25}]}");
      int httpResponseCode = http.POST("[{\"sourceId\":\"temp\",\"values\":[{\"ts\":\"2021-05-10T23:00:03Z\",\"value\":\"" + String(temp) + "\"}]},{\"sourceId\":\"audio\",\"values\":[{\"ts\":\"2021-05-07T23:00:03Z\",\"value\":\"" + String(micVal) + "\"}]}]");
      Serial.println("Sending data 1.Temp: " + String(temp) + " micValue: " + String(micVal) + "");
      Serial.print("Post data resp code: ");
      Serial.println(httpResponseCode);
      String payload2 = http.getString();
      JSONVar myobject2 = JSON.parse(payload2);
      Serial.println(myobject2);
    }
    else {
      Serial.println("Could not get access token check creds!!");
    }
  }
  else {
    Serial.println("WIFI disconnected!!");
  }
}

void receiveLoRa() {
  //Serial.println("Ready to receiver packets...");
  int packetSize = LoRa.parsePacket();
  if (packetSize > 0) {
    Serial.print("Received Packet: '");
    while (LoRa.available()) {
      Serial.print((char)LoRa.read());
    }
    //print the rssi
    Serial.print("' with rssi: ");
    Serial.println(LoRa.packetRssi());
  }
}
