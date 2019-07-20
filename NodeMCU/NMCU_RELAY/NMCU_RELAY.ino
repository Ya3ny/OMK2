// NMCU_RELAY 

#include <ESP8266WiFi.h>
#include <PubSubClient.h>


// Change the credentials below, so your ESP8266 connects to your router
const char* ssid = "C170512D5CFB";
const char* password = "9053393116";

// Change the variable to your Raspberry Pi IP address, so it connects to your MQTT broker
const char* mqtt_server = "192.168.0.215";

// Initializes the espClient. You should change the espClient name if you have multiple ESPs running in your home automation system
WiFiClient NMCU_RELAY;
PubSubClient client(NMCU_RELAY);

const int R1 = 16;   // D0 on ESP-12E NodeMCU board
const int R2 = 5;    // D1 on ESP-12E NodeMCU board
const int R3 = 4;    // D2 on ESP-12E NodeMCU board
const int R4 = 0;    // D3 on ESP-12E NodeMCU board
const int R5 = 2;    // D4 on ESP-12E NodeMCU board
const int R6 = 14;   // D5 on ESP-12E NodeMCU board
const int R7 = 12;   // D6 on ESP-12E NodeMCU board
const int R8 = 13;   // D8 on ESP-12E NodeMCU board


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

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic NMCU_RELAY/R#, you check if the message is either on or off. Changes R# GPIO state according to the message
  if(topic=="NMCU_RELAY/R1"){
      Serial.print("Changing R1 state to: ");
      if(messageTemp == "on"){ digitalWrite(R1, LOW); Serial.print("ON"); }
      else if(messageTemp == "off"){digitalWrite(R1, HIGH); Serial.print("OFF");}
  }
  if(topic=="NMCU_RELAY/R2"){
      Serial.print("Changing R2 state to: ");
      if(messageTemp == "on"){ digitalWrite(R2, LOW); Serial.print("ON"); }
      else if(messageTemp == "off"){digitalWrite(R2, HIGH); Serial.print("OFF");}
  }
  if(topic=="NMCU_RELAY/R3"){
      Serial.print("Changing R3 state to: ");
      if(messageTemp == "on"){ digitalWrite(R3, LOW); Serial.print("ON"); }
      else if(messageTemp == "off"){digitalWrite(R3, HIGH); Serial.print("OFF");}
  }
  if(topic=="NMCU_RELAY/R4"){
      Serial.print("Changing R4 state to: ");
      if(messageTemp == "on"){ digitalWrite(R4, LOW); Serial.print("ON"); }
      else if(messageTemp == "off"){digitalWrite(R4, HIGH); Serial.print("OFF");}
  }
  if(topic=="NMCU_RELAY/R5"){
      Serial.print("Changing R5 state to: ");
      if(messageTemp == "on"){ digitalWrite(R5, LOW); Serial.print("ON"); }
      else if(messageTemp == "off"){digitalWrite(R5, HIGH); Serial.print("OFF");}
  }
  if(topic=="NMCU_RELAY/R6"){
      Serial.print("Changing R6 state to: ");
      if(messageTemp == "on"){ digitalWrite(R6, LOW); Serial.print("ON"); }
      else if(messageTemp == "off"){digitalWrite(R6, HIGH); Serial.print("OFF");}
  }
  if(topic=="NMCU_RELAY/R7"){
      Serial.print("Changing R7 state to: ");
      if(messageTemp == "on"){ digitalWrite(R7, LOW); Serial.print("ON"); }
      else if(messageTemp == "off"){digitalWrite(R7, HIGH); Serial.print("OFF");}
  }
  if(topic=="NMCU_RELAY/R8"){
      Serial.print("Changing R8 state to: ");
      if(messageTemp == "on"){ digitalWrite(R8, LOW); Serial.print("ON"); }
      else if(messageTemp == "off"){digitalWrite(R8, HIGH); Serial.print("OFF");}
  }
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
    if (client.connect("NMCU_RELAY")) {
      Serial.println("connected");  
      // Subscribe or resubscribe to a topic
      // You can subscribe to more topics (to control more LEDs in this example)
      client.subscribe("NMCU_RELAY/R1");
      client.subscribe("NMCU_RELAY/R2");
      client.subscribe("NMCU_RELAY/R3");
      client.subscribe("NMCU_RELAY/R4");
      client.subscribe("NMCU_RELAY/R5");
      client.subscribe("NMCU_RELAY/R6");
      client.subscribe("NMCU_RELAY/R7");
      client.subscribe("NMCU_RELAY/R8");
    }
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// The setup function sets your ESP GPIOs to Outputs, starts the serial communication at a baud rate of 115200
// Sets your mqtt broker and sets the callback function
// The callback function is what receives messages and actually controls the LEDs
void setup() {
  pinMode(R1, OUTPUT);
  pinMode(R2, OUTPUT);
  pinMode(R3, OUTPUT);
  pinMode(R4, OUTPUT);
  pinMode(R5, OUTPUT);
  pinMode(R6, OUTPUT);
  pinMode(R7, OUTPUT);
  pinMode(R8, OUTPUT);


  digitalWrite(R1, HIGH);
  digitalWrite(R2, HIGH);
  digitalWrite(R3, HIGH);
  digitalWrite(R4, HIGH);
  digitalWrite(R5, HIGH);
  digitalWrite(R6, HIGH);
  digitalWrite(R7, HIGH);
  digitalWrite(R8, HIGH);
  
  Serial.begin(115200);
  
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

}

// For this project, you don't need to change anything in the loop function. Basically it ensures that you ESP is connected to your broker
void loop() {

  if (!client.connected()) {
    reconnect();
  }
  if(!client.loop()) {
    client.connect("NMCU_RELAY");
  }
} 
