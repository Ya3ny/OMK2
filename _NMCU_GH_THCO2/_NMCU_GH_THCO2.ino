/*
 * projetsdiy.fr - diyprojects.io (december 2017) 
 */
//#include <Wire.h>
#include <SoftwareSerial.h>
// For Arduino or ESP32 (Espressif SDK must be installed) 
//#include <WiFi.h>
//#include <HTTPClient.h>
// Pour une carte ESP8266 | For ESP8266 development board
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

// Uncomment one of the lines below for whatever DHT sensor type you're using!
#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321


//  DO NOT USE D3{0}, D4{2}, D8{15} WITHOUT A PULLUP/PULLDOWN

const int DHTPin = 5;       // D1
const int dPWR = 4;         // D2

// Initialize DHT sensor.
DHT dht(DHTPin, DHTTYPE);

long now = millis();
long lastMeasure = 0;

#define INTERVAL 10000
const int MH_Z19_RX = 3;   // D5
const int MH_Z19_TX = 1;   // D6

#define resetPin 16  // D0

long previousMillis = 0;

SoftwareSerial co2Serial(MH_Z19_RX, MH_Z19_TX); // define MH-Z19

// Change the credentials below, so your ESP8266 connects to your router
const char* ssid = "SSID";
const char* password = "PASSWORD";

// Change the variable to your IP address, so it connects to your MQTT broker
const char* mqtt_server = "0.0.0.0";


#define IDX_mhz19   26

WiFiClient NMCU_GH_AIR;
PubSubClient client(NMCU_GH_AIR);

// Don't change the function below. This functions connects your ESP8266 to your router
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected - ESP IP address: ");
  Serial.println(WiFi.localIP());
}

int inByte;
float celsius = 25.0, Vph;
boolean PCB_check = false;

char swv[]={"180522AS"};

//
//--------------------------------------------------------
//
//  get Real Time and Date
//
 
void callback(String topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  if(topic=="sense/reset"){
    if(messageTemp == "reset"){
      Serial.println("RESET");
      digitalWrite(resetPin,LOW);
    }
  }
}

// This functions reconnects your ESP8266 to your MQTT broker
// Change the function below if you want to subscribe to more topics with your ESP8266 
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("NMCU_GH_AIR")) {
      Serial.println("connected");
      // Subscribe or resubscribe to a topic
      // You can subscribe to more topics (to control more LEDs in this example)
      client.subscribe("sense/CO2");
      client.subscribe("sense/reset");
      
      
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  digitalWrite(resetPin,HIGH);
  pinMode(resetPin, OUTPUT);
  pinMode(dPWR,OUTPUT);
  digitalWrite(dPWR,HIGH);

  dht.begin();
  
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  
  Serial.begin(9600);
  co2Serial.begin(9600); //Init sensor MH-Z19(14)

  unsigned long previousMillis = millis();
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  if(!client.loop())
    client.connect("NMCU_GH_AIR");

  now = millis();
  if (now - lastMeasure > 10000) {
    lastMeasure = now;

    Serial.print("Requesting CO2 concentration...");
    int ppm = readCO2();
    Serial.println("  PPM = " + String(ppm));
  
    static char ppmTemp[7];
    dtostrf(ppm, 6, 0, ppmTemp);
  
    client.publish("sense/GH/CO2", ppmTemp);

    float h = dht.readHumidity();
    float t = dht.readTemperature();
    
    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t)) {
      Serial.println("Failed to read from DHT sensor!");
    }

    // Computes temperature values in Celsius
    float hic = dht.computeHeatIndex(t, h, false);

    static char temperatureTemp[7];
    dtostrf(t, 6, 1, temperatureTemp);
    //dtostrf(hic, 6, 1, temperatureTemp);

    static char humidityTemp[7];
    dtostrf(h, 6, 0, humidityTemp);

    Serial.print("Humidity: ");
    Serial.print(h);
    
    Serial.print(" \t AIR [T]: ");
    Serial.print(t);
    Serial.println(" *C");
    
    client.publish("sense/GH/T", temperatureTemp);
    client.publish("sense/GH/H", humidityTemp);
  }
}

int readCO2() {
  // From original code https://github.com/jehy/arduino-esp8266-mh-z19-serial
  byte cmd[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
  // command to ask for data
  byte response[9]; // for answer

  co2Serial.write(cmd, 9); //request PPM CO2

  // The serial stream can get out of sync. The response starts with 0xff, try to resync.
  while (co2Serial.available() > 0 && (unsigned char)co2Serial.peek() != 0xFF) {
    co2Serial.read();
  }

  memset(response, 0, 9);
  co2Serial.readBytes(response, 9);

  if (response[1] != 0x86)
  {
    Serial.println("Invalid response from co2 sensor!");
    return -1;
  }

  byte crc = 0;
  for (int i = 1; i < 8; i++) {
    crc += response[i];
  }
  crc = 255 - crc + 1;

  if (response[8] == crc) {
    int responseHigh = (int) response[2];
    int responseLow = (int) response[3];
    int ppm = (256 * responseHigh) + responseLow;
    return ppm;
  } else {
    Serial.println("CRC error!");
    return -1;
  }
}
