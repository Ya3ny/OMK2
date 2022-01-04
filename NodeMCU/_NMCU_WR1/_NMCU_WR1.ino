// NMCU_WR1

// Using HC-SR04

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include <OneWire.h>
#include <DallasTemperature.h>

// Change the credentials below, so your ESP8266 connects to your router
const char* ssid = "SSID";
const char* password = "PASSWORD";

// Change the variable to your IP address, so it connects to your MQTT broker
const char* mqtt_server = "0.0.0.0";


WiFiClient NMCU_WR1;
PubSubClient client(NMCU_WR1);


// Timers auxiliar variables
long now = millis();
long lastMeasure = 0;


int echo = 5;     // D1;
int trig = 4;  // D2;
int data;

#define ONE_WIRE_BUS 14 //D5(GRN)  5V(RED)  GND(YLW)

OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire);


// Don't change the function below. This functions connects your ESP8266 to your router
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
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

// This functions is executed when some device publishes a message to a topic that your ESP8266 is subscribed to
// Change the function below to add logic to your program, so when a device publishes a message to a topic that 
// your ESP8266 is subscribed you can actually do something
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
/*
  if(topic=="sense/reset"){
    if(messageTemp == "reset"){
      Serial.println("RESET");
      digitalWrite(resetPin,LOW);
    }
  }
*/
  Serial.println();
}

// This functions reconnects your ESP8266 to your MQTT broker
// Change the function below if you want to subscribe to more topics with your ESP8266 
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    /*
     YOU MIGHT NEED TO CHANGE THIS LINE, IF YOU'RE HAVING PROBLEMS WITH MQTT MULTIPLE CONNECTIONS
     To change the ESP device ID, you will have to give a new name to the ESP8266.
     Here's how it looks:
       if (client.connect("ESP8266Client")) {
     You can do it like this:
       if (client.connect("ESP1_Office")) {
     Then, for the other ESP:
       if (client.connect("ESP2_Garage")) {
      That should solve your MQTT multiple connections problem
    */
    if (client.connect("NMCU_WR1")) {
      Serial.println("connected");  
      // Subscribe or resubscribe to a topic
      // You can subscribe to more topics (to control more LEDs in this example)
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
  Serial.begin(115200);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  sensors.begin();

  pinMode(trig, OUTPUT); 
  pinMode(echo, INPUT); 
}


void loop() {

  if (!client.connected()) {
    reconnect();
  }
  if(!client.loop())
    client.connect("NMCU_WR1");

  now = millis();
  if (now - lastMeasure > 10000) {
    lastMeasure = now;

    digitalWrite(trig, LOW);  
    delayMicroseconds(2); 
    digitalWrite(trig, HIGH);
    delayMicroseconds(10); 
    digitalWrite(trig, LOW);
    long duration = pulseIn(echo, HIGH);
    data = (duration/2) / 29.09;
    
    int water_level_1 = data;   // calibrate on NodeRED side

    // Check if any reads failed and exit early (to try again).
    if (isnan(water_level_1)) {
      Serial.println("Failed to read from differential pressure sensor!");
    return;
    }

    static char WR1_L_msg[7];
    dtostrf(water_level_1, 6, 1, WR1_L_msg);

    Serial.print("Distance: ");
    Serial.println(data);


    sensors.requestTemperatures();

    float WR1_T = sensors.getTempCByIndex(0);

    static char WR1_T_msg[7];
    dtostrf(WR1_T, 6, 1, WR1_T_msg);

    Serial.print("WR1_T: ");
    Serial.println(WR1_T);

    // Publishes sensor values
    client.publish("sense/GH/WR1_L", WR1_L_msg);
    client.publish("sense/GH/WR1_T", WR1_T_msg);
  }
}
