#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include "driver/i2s.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include "time.h"
#include <SPI.h>
#include <LoRa.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

//wifi,server and client device creds
//const char* ssid  = "THE DEVELOPER";
//const char* password = "MANAGEMENT2";
const char* ssid  = "dan";
const char* password = "dandandandan";
const char* accessUrl = "https://anabi-smartsystems.com/api/token";
const char* dataUrl = "https://anabi-smartsystems.com/api/data";
const char* clientId = "YF9GDOGOXdcVM23UI56mm0wtXv9TsCMj";
const char* clientSecret = "5FHyIyVQW6O536waXzUGoTI69dE2gdgKBcem4CnOUyepSJth8TRH56HsNzQH3JSt";
const char* ntpServer = "pool.ntp.org";
long lastSendTime = 0;
int sendInterval = 2000;//send to server interval
//source ids
const char* mainTemp = "mainTemp";//mainTemp
const char* mainAudio = "mainAudio";//mainAudio
const char* auxTemp = "auxTemp";
const char* auxHum = "auxHum";
const char* auxWeight = "auxWeight";

//i2s
const i2s_port_t I2S_PORT = I2S_NUM_0; //i2s peripheral

//ds18b20
const int oneWireBus = 21;

//time
const long gmtoffsetSec = 3;
const int daylightOffsetSec = 10800;
// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
// Variables to save date and time
String formattedDate;

//LoRa
#define ss 5
#define rst 14
#define dio0 2
byte msgCount = 0;            // count of incoming messages
byte localAddress = 0x02;     // address of this device
int loraTimeout = 5000; //Lora packet wait period
unsigned long timeNow = 0;
bool loraReceived = false;
String auxPayload = "";

//ds18b20
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
  // Initialize a NTPClient to get time
  timeClient.begin();
  timeClient.setTimeOffset(10800);
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
  Serial.println("LoRa Receiver initialized");
  if (!LoRa.begin(915E6)) {
    Serial.println("Starting LoRa failed");
    while (1);
  }

}

void loop() {
  //give precedensce to LoRa msg receive
  timeNow = millis();
  //onReceive(LoRa.parsePacket());
  //give the LoRa a 5 seconds window to receive a packet from the aux board
  while (millis() < timeNow + loraTimeout) {
    //wait to receive loRa packet
    onReceive(LoRa.parsePacket());
    if (loraReceived) {
      break;
    }
  }
  if (!loraReceived) {
    Serial.println("Failed to receive payload from aux board please check it!!");
  }
  loraReceived = false;
  float curnTemp = getSensorTemperature();
  float micVal = normalizeMicdata(getMicData());
  String timeStamp = getTimestamp();
  sendMainData(getAt(), curnTemp, micVal, timeStamp);
  if (auxPayload.length() > 0) {
    //decode the data
    int n = auxPayload.length();
    char sensorArray[n + 1];
    strcpy(sensorArray, auxPayload.c_str());

    char *Words[10];
    byte word_count = 0; //number of words
    char * item = strtok (sensorArray, " ,");
    while (item != NULL) {
      if (word_count >= 10) {
        break;
      }
      Words[word_count] = item;
      item = strtok (NULL, " ,"); //getting subsequence word
      word_count++;
    }

    sendAuxData(getAt(), Words[2], Words[1], Words[0], timeStamp);

  }
  auxPayload = "";
}

int32_t getMicData() {
  int32_t sample = 0;
  int bytes_read = i2s_pop_sample(I2S_PORT, (char *)&sample, portMAX_DELAY); // no timeout
  if (bytes_read > 0) {
    return sample;
  }
}
//0-1dB
float normalizeMicdata(int32_t sample) {
  //Range of a 32 bit integer : -2,147,483,648 to +2,147,483,647
  //int audioVal = map(sample,-2147483648,2147483647,1, 10); //this doesnt work will research on it later
  int32_t audioVal = constrain(sample, -2147483648, 2147483647);
  float audVal = map(audioVal, -2147483648, 2147483647, 0, 1);
  float aud = audVal / 1000000000;
  float norm = map(aud, -2, 2, 0, 10);
  float out = norm / 10;
  return out;
}

//clean the access token
String cleanAt(String at) {
  int fin = at.length() - 1;
  String out = at.substring(1, fin);
  return out;
}

float getSensorTemperature() {
  sensors.requestTemperatures();
  float temperatureC = sensors.getTempCByIndex(0)-7.00;//compensate for sensor proximity to board
  return temperatureC;
}

String getTimestamp() {
  while (!timeClient.update()) {
    timeClient.forceUpdate();
  }
  formattedDate = timeClient.getFormattedDate();
  return formattedDate;
}
String getAt() {
  if (WiFi.status() == WL_CONNECTED) {
    //get the access token
    HTTPClient http;
    http.begin(accessUrl);
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.POST("{\"client_id\":\"" + String(clientId) + "\",\"client_secret\":\"" + String(clientSecret) + "\",\"audience\":\"sams-dwh-web-api\",\"grant_type\":\"client_credentials\"}");
    if (httpResponseCode == 200) {
      String payload = http.getString();
      JSONVar myobject = JSON.parse(payload);
      JSONVar accessToken = myobject["access_token"];
      String at = JSON.stringify(accessToken);
      http.end();
      return cleanAt(at);
    }
    else {
      Serial.println("Could not get access token check creds!!");
    }
  }
  else {
    Serial.println("WIFI disconnected!!");
  }
}
void sendMainData(String at, float temp, int32_t micVal, String timeStamp) {
  if (WiFi.status() == WL_CONNECTED) {
    //post data to anabi server
    Serial.println("Sending main board data");
    HTTPClient http;
    http.begin(dataUrl);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", "Bearer " + at + "");
    int httpResponseCode = http.POST("[{\"sourceId\":\"" + String(mainTemp) + "\",\"values\":[{\"ts\":\"" + timeStamp + "\",\"value\":\"" + String(temp) + "\"}]},{\"sourceId\":\"" + String(mainAudio) + "\",\"values\":[{\"ts\":\"" + timeStamp + "\",\"value\":\"" + String(micVal) + "\"}]}]");
    Serial.println("Sending data 1.Temp: " + String(temp) + " micValue: " + String(micVal) + "");
    Serial.print("Post data resp code: ");
    Serial.println(httpResponseCode);
    String payload2 = http.getString();
    JSONVar myobject2 = JSON.parse(payload2);
    Serial.println(myobject2);
  }

  else {
    Serial.println("WIFI disconnected!!");
  }
}

void sendAuxData(String at, String temp, String hum, String weight, String timeStamp) {
  if (WiFi.status() == WL_CONNECTED) {
    //post data to anabi server
    Serial.println("Sending aux board data");
    HTTPClient http;
    http.begin(dataUrl);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", "Bearer " + at + "");
    int httpResponseCode = http.POST("[{\"sourceId\":\"" + String(auxTemp) + "\",\"values\":[{\"ts\":\"" + timeStamp + "\",\"value\":\"" + String(temp) + "\"}]},{\"sourceId\":\"" + String(auxHum) + "\",\"values\":[{\"ts\":\"" + timeStamp + "\",\"value\":\"" + String(hum) + "\"}]},{\"sourceId\":\"" + String(auxWeight) + "\",\"values\":[{\"ts\":\"" + timeStamp + "\",\"value\":\"" + String(weight) + "\"}]}]");
    Serial.println("Sending data 1.Temp: " + String(auxTemp) + " Humidity: " + String(hum) + "weight: " + String(weight) + "");
    Serial.print("Post data resp code: ");
    Serial.println(httpResponseCode);
    String payload2 = http.getString();
    JSONVar myobject2 = JSON.parse(payload2);
    Serial.println(myobject2);
  }

  else {
    Serial.println("WIFI disconnected!!");
  }
}
void onReceive(int packetSize) {
  if (packetSize == 0) return;          // if there's no packet, return

  // read packet header bytes:
  int recipient = LoRa.read();          // recipient address
  byte sender = LoRa.read();            // sender address
  byte incomingMsgId = LoRa.read();     // incoming msg ID
  byte incomingLength = LoRa.read();    // incoming msg length

  String incoming = "";

  while (LoRa.available()) {
    incoming += (char)LoRa.read();
  }

  if (incomingLength != incoming.length()) {   // check length for error
    Serial.println("error: message length does not match length");
    return;                             // skip rest of function
  }

  // if the recipient isn't this device or broadcast,
  if (recipient != localAddress && recipient != 0xFF) {
    Serial.println("This message is not for me.");
    return;                             // skip rest of function
  }
  auxPayload = incoming;

  Serial.println("Received from: 0x" + String(sender, HEX));
  Serial.println("Sent to: 0x" + String(recipient, HEX));
  Serial.println("Message ID: " + String(incomingMsgId));
  Serial.println("Message length: " + String(incomingLength));
  Serial.println("Message: " + incoming);
  Serial.println("RSSI: " + String(LoRa.packetRssi()));
  Serial.println("Snr: " + String(LoRa.packetSnr()));
  Serial.println();
  //send the aux data
  String timeStamp = getTimestamp();
  //sendAuxData(getAt(),Words[2], Words[1],Words[0], timeStamp);
  loraReceived = true;
}
