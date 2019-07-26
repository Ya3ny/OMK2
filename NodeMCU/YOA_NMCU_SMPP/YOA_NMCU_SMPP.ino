// NMCU_SMPP 
// MQTT comments credit:  https://randomnerdtutorials.com/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Change the credentials below, so your ESP8266 connects to your router
const char* ssid = "REPLACE_WITH_YOUR_SSID";
const char* password = "REPLACE_WITH_YOUR_PASSWORD";

// Change the variable to your Raspberry Pi IP address, so it connects to your MQTT broker
const char* mqtt_server = "REPLACE_WITH_YOUR_RPI_IP_ADDRESS";

// Initializes the espClient. You should change the espClient name if you have multiple ESPs running in your home automation system
WiFiClient NMCU_SMPP;
PubSubClient client(NMCU_SMPP);


/* SMPP */
unsigned long int steps_A;
unsigned long int steps_B;
unsigned long int steps_C;
unsigned long int steps_D;

int a;
int b;
int c;
int d;

int mVal;

float A_dose;
float B_dose;
float C_dose;
float D_dose;

int SPML_N = 75;
int SPML_D = 1667;

char user_input;
int state;

#define EN_P 16  // D0 on ESP-12E NodeMCU board

#define EN_A 5   // D1
#define EN_B 4   // D2
#define EN_C 0   // D3
#define EN_D 2   // D4

#define stp_A 13  // D7
#define dir_A 15  // D8



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

  if(topic=="NMCU_SMPP/EN_P"){
      Serial.print("Changing EN_P state to: ");
      if(messageTemp == "on"){ digitalWrite(EN_P, LOW); Serial.println("ON"); }
      else if(messageTemp == "off"){ digitalWrite(EN_P, HIGH); Serial.println("OFF"); }
  }
  
  if(topic=="NMCU_SMPP/N1"){
    if(messageTemp == "dose"){
      resetDPins();
      Serial.println("Dose N1\n");
      client.publish("NMCU_RELAY/R5","on");
      digitalWrite(EN_A,LOW);
      delay(1000);;
      Dose_A();
    }
    else if(messageTemp == "whatdoseN1"){
      Serial.print("N1 dose: "); Serial.print(A_dose); Serial.println(" ml\n");
    }
    else{
      A_dose = messageTemp.toFloat(); steps_A = A_dose*SPML_N;
      Serial.print("N1 dose: "); Serial.print(A_dose); Serial.println(" ml\n");
    }
  }
  
  if(topic=="NMCU_SMPP/N2"){
    if(messageTemp == "dose"){
      Serial.println("Dose N2\n");
      client.publish("NMCU_RELAY/R5","on");
      digitalWrite(EN_B,LOW);
      delay(1000);;
      Dose_B();
    }
    else if(messageTemp == "whatdoseN2"){
      Serial.print("N2 dose: "); Serial.print(B_dose); Serial.println(" ml\n");
    }
    else{
      B_dose = messageTemp.toFloat(); steps_B = B_dose*SPML_N;
      Serial.print("N2 dose: "); Serial.print(B_dose); Serial.println(" ml\n");
    }
  }
  
  if(topic=="NMCU_SMPP/N3"){
    if(messageTemp == "dose"){
      Serial.println("Dose N3\n");
      client.publish("NMCU_RELAY/R5","on");
      digitalWrite(EN_C,LOW);
      delay(1000);;
      Dose_C();
    }
    else if(messageTemp == "whatdoseN3"){
      Serial.print("N3 dose: "); Serial.print(C_dose); Serial.println(" ml\n");
    }
    else{
      C_dose = messageTemp.toFloat(); steps_C = C_dose*SPML_N;
      Serial.print("N3 dose: "); Serial.print(C_dose); Serial.println(" ml\n");
    }
  }

  if(topic=="NMCU_SMPP/pHd"){
    if(messageTemp == "dose"){
      Serial.println("Dose pHd\n");
      client.publish("NMCU_RELAY/R5","on");
      digitalWrite(EN_D,LOW);
      delay(1000);;
      Dose_D();
    }
    else if(messageTemp == "whatdosepHd"){
      Serial.print("pHd dose: "); Serial.print(D_dose); Serial.println(" ml\n");
    }
    else{
      D_dose = messageTemp.toFloat(); steps_D = D_dose*SPML_D;
      Serial.print("pHd dose: "); Serial.print(D_dose); Serial.println(" ml\n");
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
       if (client.connect("ESEN_P_Office")) {
     Then, for the other ESP:
       if (client.connect("ESEN_P_Garage")) {
      That should solve your MQTT multiple connections problem
    */
    if (client.connect("NMCU_SMPP")) {
      Serial.println("connected");  
      // Subscribe or resubscribe to a topic
      // You can subscribe to more topics (to control more LEDs in this example)
      client.subscribe("NMCU_SMPP/EN_P");
      client.subscribe("NMCU_SMPP/N1");
      client.subscribe("NMCU_SMPP/N2");
      client.subscribe("NMCU_SMPP/N3");
      client.subscribe("NMCU_SMPP/pHd");
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
// The callback function is what receives messages and actually controls the LEDs
void setup() {
  Serial.begin(115200);
  
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  
  pinMode(EN_P, OUTPUT);
  digitalWrite(EN_P, HIGH);

  pinMode(stp_A, OUTPUT);
  pinMode(dir_A, OUTPUT);
  
  pinMode(EN_A, OUTPUT);
  pinMode(EN_B, OUTPUT);
  pinMode(EN_C, OUTPUT);
  pinMode(EN_D, OUTPUT);

  resetDPins();
}

// For this project, you don't need to change anything in the loop function. Basically it ensures that you ESP is connected to your broker
void loop() {

  if (!client.connected()) {
    reconnect();
  }
  if(!client.loop()) {
    client.connect("NMCU_SMPP");
  }
}


void Dose_A(){
  Serial.print("Moving forward ");Serial.print(steps_A);Serial.println(" steps.");
  digitalWrite(dir_A, LOW); //Pull direction pin low to move "forward"
  for(a= 1; a<steps_A; a++){
    digitalWrite(stp_A,HIGH); //Trigger one step forward
    delay(1);
    digitalWrite(stp_A,LOW); //Pull step pin low so it can be triggered again
    delay(10);
  }
  resetDPins();
  //client.publish("NMCU_RELAY/R5","off");
  Serial.println("Enter new option");
  Serial.println();
}

void Dose_B(){
  Serial.print("Moving forward ");Serial.print(steps_B);Serial.println(" steps.");
  digitalWrite(dir_A, LOW); //Pull direction pin low to move "forward"
  for(b= 1; b<steps_B; b++){
    digitalWrite(stp_A,HIGH); //Trigger one step forward
    delay(1);
    digitalWrite(stp_A,LOW); //Pull step pin low so it can be triggered again
    delay(10);
  }
  resetDPins();
  //client.publish("NMCU_RELAY/R5","off");
  Serial.println("Enter new option");
  Serial.println();
}

void Dose_C(){
  Serial.print("Moving forward ");Serial.print(steps_C);Serial.println(" steps.");
  digitalWrite(dir_A, LOW); //Pull direction pin low to move "forward"
  for(c= 1; c<steps_C; c++){
    digitalWrite(stp_A,HIGH); //Trigger one step forward
    delay(1);
    digitalWrite(stp_A,LOW); //Pull step pin low so it can be triggered again
    delay(10);
  }
  resetDPins();
  //client.publish("NMCU_RELAY/R5","off");
  Serial.println("Enter new option");
  Serial.println();
}

void Dose_D(){
  Serial.print("Moving forward ");Serial.print(steps_D);Serial.println(" steps.");
  digitalWrite(dir_A, LOW); //Pull direction pin low to move "forward"
  for(d= 1; d<steps_D; d++){
    digitalWrite(stp_A,HIGH); //Trigger one step forward
    delay(1);
    digitalWrite(stp_A,LOW); //Pull step pin low so it can be triggered again
    delay(1);
  }
  resetDPins();
  client.publish("NMCU_RELAY/R5","off");
  Serial.println("Enter new option");
  Serial.println();
}


void resetDPins(){
  digitalWrite(stp_A, LOW);
  
  digitalWrite(EN_A, HIGH);
  digitalWrite(EN_B, HIGH);
  digitalWrite(EN_C, HIGH);
  digitalWrite(EN_D, HIGH);
}

bool inRange(int val, int minimum, int maximum)
{
  return ((minimum < val) && (val < maximum));
}
