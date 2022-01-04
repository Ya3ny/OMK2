// NMCU_R2 

#include <ESP8266WiFi.h>
#include <PubSubClient.h>


// Change the credentials below, so your ESP8266 connects to your router
const char* ssid = "SSID";
const char* password = "PASSWORD";

// Change the variable to your IP address, so it connects to your MQTT broker
const char* mqtt_server = "0.0.0.0";


// Initializes the espClient. You should change the espClient name if you have multiple ESPs running in your home automation system
WiFiClient NMCU_R2;
PubSubClient client(NMCU_R2);

//  DO NOT USE D3{0}, D4{2}, D8{15} WITHOUT A PULLUP/PULLDOWN

const int C1 = 404;
const int C2 = 404;
const int C3 = 16;   // D0 on ESP-12E NodeMCU board
const int C4 = 5;    // D1 on ESP-12E NodeMCU board
const int C5 = 4;    // D2 on ESP-12E NodeMCU board
const int C6 = 14;   // D5 on ESP-12E NodeMCU board
const int C7 = 12;   // D6 on ESP-12E NodeMCU board
const int C8 = 13;   // D7 on ESP-12E NodeMCU board


const char ON = LOW;
const char OFF = HIGH;


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

  // If a message is received on the topic NMCU_R2/C#, you check if the message is either on or off. Changes R# GPIO state according to the message
  if(topic=="NMCU_R2/C1"){
      Serial.print("Changing C1 state to: ");
      if(messageTemp == "on"){ digitalWrite(C1, ON); Serial.print("ON"); }
      else if(messageTemp == "off"){digitalWrite(C1, OFF); Serial.print("OFF");}
  }
  if(topic=="NMCU_R2/C2"){
      Serial.print("Changing C2 state to: ");
      if(messageTemp == "on"){ digitalWrite(C2, ON); Serial.print("ON"); }
      else if(messageTemp == "off"){digitalWrite(C2, OFF); Serial.print("OFF");}
  }
  if(topic=="NMCU_R2/C3"){
      Serial.print("Changing C3 state to: ");
      if(messageTemp == "on"){ digitalWrite(C3, ON); Serial.print("ON"); }
      else if(messageTemp == "off"){digitalWrite(C3, OFF); Serial.print("OFF");}
  }
  if(topic=="NMCU_R2/C4"){
      Serial.print("Changing C4 state to: ");
      if(messageTemp == "on"){ digitalWrite(C4, ON); Serial.print("ON"); }
      else if(messageTemp == "off"){digitalWrite(C4, OFF); Serial.print("OFF");}
  }
  if(topic=="NMCU_R2/C5"){
      Serial.print("Changing C5 state to: ");
      if(messageTemp == "on"){ digitalWrite(C5, ON); Serial.print("ON"); }
      else if(messageTemp == "off"){digitalWrite(C5, OFF); Serial.print("OFF");}
  }
  if(topic=="NMCU_R2/C6"){
      Serial.print("Changing C6 state to: ");
      if(messageTemp == "on"){ digitalWrite(C6, ON); Serial.print("ON"); }
      else if(messageTemp == "off"){digitalWrite(C6, OFF); Serial.print("OFF");}
  }
  if(topic=="NMCU_R2/C7"){
      Serial.print("Changing C7 state to: ");
      if(messageTemp == "on"){ digitalWrite(C7, ON); Serial.print("ON"); }
      else if(messageTemp == "off"){digitalWrite(C7, OFF); Serial.print("OFF");}
  }
  if(topic=="NMCU_R2/C8"){
      Serial.print("Changing C8 state to: ");
      if(messageTemp == "on"){ digitalWrite(C8, ON); Serial.print("ON"); }
      else if(messageTemp == "off"){digitalWrite(C8, OFF); Serial.print("OFF");}
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
    if (client.connect("NMCU_R2")) {
      Serial.println("connected");  
      // Subscribe or resubscribe to a topic
      // You can subscribe to more topics (to control more LEDs in this example)
      client.subscribe("NMCU_R2/C1");
      client.subscribe("NMCU_R2/C2");
      client.subscribe("NMCU_R2/C3");
      client.subscribe("NMCU_R2/C4");
      client.subscribe("NMCU_R2/C5");
      client.subscribe("NMCU_R2/C6");
      client.subscribe("NMCU_R2/C7");
      client.subscribe("NMCU_R2/C8");
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
  pinMode(C1, OUTPUT);
  pinMode(C2, OUTPUT);
  pinMode(C3, OUTPUT);
  pinMode(C4, OUTPUT);
  pinMode(C5, OUTPUT);
  pinMode(C6, OUTPUT);
  pinMode(C7, OUTPUT);
  pinMode(C8, OUTPUT);

  delay(500);

  digitalWrite(C1, ON);
  digitalWrite(C2, ON);
  digitalWrite(C3, ON);
  digitalWrite(C4, ON);
  digitalWrite(C5, ON);
  digitalWrite(C6, ON);
  digitalWrite(C7, ON);
  digitalWrite(C8, ON);

  delay(100);

  digitalWrite(C1, OFF);
  digitalWrite(C2, OFF);
  digitalWrite(C3, OFF);
  digitalWrite(C4, OFF);
  digitalWrite(C5, OFF);
  digitalWrite(C6, OFF);
  digitalWrite(C7, OFF);
  digitalWrite(C8, OFF);

  
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
    client.connect("NMCU_R2");
  }
} 
