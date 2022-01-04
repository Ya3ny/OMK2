/*****
 
 All the resources for this project:
 http://randomnerdtutorials.com/
 
*****/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>


// Change the credentials below, so your ESP8266 connects to your router
const char* ssid = "SSID";
const char* password = "PASSWORD";

// Change the variable to your Raspberry Pi IP address, so it connects to your MQTT broker
const char* mqtt_server = "0.0.0.0";

// Initializes the espClient. You should change the espClient name if you have multiple ESPs running in your home automation system
WiFiClient NMCU_GH_SML_1;
PubSubClient client(NMCU_GH_SML_1);

//  DO NOT USE D3{0}, D4{2}, D8{15} WITHOUT A PULLUP/PULLDOWN

const int resetPin = 16;  // D0 on ESP-12E NodeMCU board

const int csmls = A0;  // ESP8266 Analog Pin ADC0 = A0



// Timers auxiliar variables
long now = millis();
long lastMeasure = 0;

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

  if(topic=="sense/reset"){
    if(messageTemp == "reset"){
      Serial.println("RESET");
      digitalWrite(resetPin,LOW);
    }
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
    if (client.connect("NMCU_GH_SML_1")) {
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

// The setup function sets your ESP GPIOs to Outputs, starts the serial communication at a baud rate of 115200
// Sets your mqtt broker and sets the callback function
// The callback function is what receives messages and actually controls the device
void setup() {
  digitalWrite(resetPin,HIGH);
  pinMode(resetPin, OUTPUT);
  
  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

}

// For this project, you don't need to change anything in the loop function. Basically it ensures that you ESP is connected to your broker
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  if(!client.loop())
    client.connect("NMCU_GH_SML_1");

  now = millis();
  if (now - lastMeasure > 60000) {
    lastMeasure = now;

    int soil_moist_lvl = 1024 - analogRead(csmls);

    // Check if any reads failed and exit early (to try again).
    if (isnan(soil_moist_lvl)) {
      Serial.println("Failed to read from soil moisture sensor!");
    return;
    }

    static char moistLVL_msg[7];
    dtostrf(soil_moist_lvl, 6, 0, moistLVL_msg);

    Serial.println(soil_moist_lvl);
      
    // Publishes sensor values
    client.publish("GH/SML/S1", moistLVL_msg);
  }
} 
