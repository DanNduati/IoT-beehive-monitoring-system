#include <SPI.h>
#include <LoRa.h>
#include <dht.h>
#include "HX711.h"
#define dataPin 3 //data pin for the dht22 sensor

//dht22
dht DHT;
HX711 scale(5, 6);
//LoRa

#define dio0 4 //reconfigure the di00
//LoRa.setPins(ss, rst, dio0);
byte msgCount = 0;            // count of outgoing messages
byte localAddress = 0x01;     // address of this device
byte mainBoardAddress = 0x02;      // destination to send to
long lastSendTime = 0;        // last send time
int interval = 1000;          // interval between sends


//load sensor configs
float calibrationFactor = 40;
float weight; //weight in grams

void setup() {
  Serial.begin(9600);
  Serial.println("Anabi auxilliary board initializing...");
  scale.set_scale();
  scale.tare(); //reset the scale to 0
  long zero_factor = scale.read_average(); //Get a baseline reading
  if (!LoRa.begin(915E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
}

void loop() {
  if (millis() - lastSendTime > interval) {
    int readData = DHT.read22(dataPin);
    float temp = DHT.temperature; // Gets the values of the temperature
    float hum = DHT.humidity; // Gets the values of the humidity
    float weight = getWeight();
    Serial.print("Temperature: ");
    Serial.print(temp);
    Serial.print("  ");
    Serial.print("Humidity: ");
    Serial.print(hum);
    Serial.print("  ");
    Serial.print("Weight: ");
    Serial.println(weight);

    //package the data to be sent via LoRa
    String h, t, w ;
    h.concat(hum);
    t.concat(temp);
    w.concat(weight);
    String message = w + "," + h + "," + t;
    //send the packet
    sendMessage(message);
    Serial.println("Sending " + message);
    lastSendTime = millis();
    interval = random(2000) + 1000;    // 2-3 seconds
    
    
  }
}

float getWeight() {
  scale.set_scale(calibrationFactor);
  Serial.print("Reading: ");
  weight = scale.get_units(), 10;
  if (weight < 0)
  {
    weight = 0.00;
  }
  return weight/1000;
}

void sendMessage(String outgoing) {
  LoRa.beginPacket();                   // start packetl
  LoRa.write(mainBoardAddress);              // add destination address
  LoRa.write(localAddress);             // add sender address
  LoRa.write(msgCount);                 // add message ID
  LoRa.write(outgoing.length());        // add payload length
  LoRa.print(outgoing);                 // add payload
  LoRa.endPacket();                     // finish packet and send it
  msgCount++;                           // increment message ID
}
