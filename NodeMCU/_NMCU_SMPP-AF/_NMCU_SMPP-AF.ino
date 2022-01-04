// NMCU_SMPP 

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Change the credentials below, so your ESP8266 connects to your router
const char* ssid = "SSID";
const char* password = "PASSWORD";

// Change the variable to your IP address, so it connects to your MQTT broker
const char* mqtt_server = "0.0.0.0";


// Initializes the espClient. You should change the espClient name if you have multiple ESPs running
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
int SPML_D = 100;   // needs calibration

char user_input;
int state;

#define EN_A 5      // D1
#define EN_B 4      // D2
#define EN_C 14     // D5
#define EN_D 12     // D6
#define resetPin 16 // D0
#define stp_ALL 13  // D7
#define dir_ALL 15  // D8


// Don't change the function below. This functions connects your ESP8266 to your router
void setup_wifi() {
  delay(1000);    // og: delay(10);
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

  if(topic=="NMCU_SMPP/N1"){
    if(messageTemp == "dose"){
      resetDPins();
      Serial.println("Dosing N1\n");
      client.publish("NMCU_RELAY/R5","on");
      digitalWrite(EN_A,LOW);
      delay(1000);
      Dose_A();
    }

    if(messageTemp == "reset"){
      digitalWrite(resetPin,LOW);
    }

    else {
      A_dose = messageTemp.toFloat();
      Serial.print("Nute A dose: "); Serial.print(A_dose,2); Serial.println("ml");
    }
  }
  
  if(topic=="NMCU_SMPP/N2"){
    if(messageTemp == "dose"){
      resetDPins();
      Serial.println("Dosing N2\n");
      client.publish("NMCU_RELAY/R5","on");
      digitalWrite(EN_B,LOW);
      delay(1000);
      Dose_B();
    }

    if(messageTemp == "reset"){
      digitalWrite(resetPin,LOW);
    }

    else {
      B_dose = messageTemp.toFloat();
      Serial.print("Nute B dose: "); Serial.print(B_dose,2); Serial.println("ml");
    }
  }
  
  if(topic=="NMCU_SMPP/N3"){
    if(messageTemp == "dose"){
      resetDPins();
      Serial.println("Dosing N3\n");
      client.publish("NMCU_RELAY/R5","on");
      digitalWrite(EN_C,LOW);
      delay(1000);
      Dose_C();
    }

    if(messageTemp == "reset"){
      digitalWrite(resetPin,LOW);
    }

    else {
      C_dose = messageTemp.toFloat();
      Serial.print("Nute C dose: "); Serial.print(C_dose,2); Serial.println("ml");
    }
  }

  if(topic=="NMCU_SMPP/N4"){
    if(messageTemp == "dose"){
      resetDPins();
      Serial.println("Dosing N4\n");
      client.publish("NMCU_RELAY/R5","on");
      digitalWrite(EN_D,LOW);
      delay(1000);
      Dose_D();
    }

    if(messageTemp == "reset"){
      digitalWrite(resetPin,LOW);
    }

    else {
      D_dose = messageTemp.toFloat();
      Serial.print("Nute D dose: "); Serial.print(D_dose,2); Serial.println("ml");
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
      client.subscribe("NMCU_SMPP/N1");
      client.subscribe("NMCU_SMPP/N2");
      client.subscribe("NMCU_SMPP/N3");
      client.subscribe("NMCU_SMPP/N4");
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
  digitalWrite(resetPin,HIGH);
  pinMode(resetPin, OUTPUT);
  
  Serial.begin(115200);
  
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  
  pinMode(stp_ALL, OUTPUT);
  pinMode(dir_ALL, OUTPUT);
  
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
  steps_A = A_dose*SPML_N;
  Serial.print("Moving forward ");Serial.print(steps_A);Serial.println(" steps.");
  Serial.print("Dosing: ");Serial.print(A_dose);Serial.println(" ml of N1");
  digitalWrite(dir_ALL, LOW); //Pull direction pin low to move "forward"
  for(a= 1; a<steps_A; a++){
    digitalWrite(stp_ALL,HIGH); //Trigger one step forward
    delay(1);
    digitalWrite(stp_ALL,LOW); //Pull step pin low so it can be triggered again
    delay(5);
  }
  resetDPins();
  delay(1000);
  client.publish("NMCU_RELAY/R5","off");
  Serial.println("Enter new option");
  Serial.println();
}

void Dose_B(){
  steps_B = B_dose*SPML_N;
  Serial.print("Moving forward ");Serial.print(steps_B);Serial.println(" steps.");
  Serial.print("Dosing: ");Serial.print(B_dose);Serial.println(" ml of nute B");
  digitalWrite(dir_ALL, LOW); //Pull direction pin low to move "forward"
  for(b= 1; b<steps_B; b++){
    digitalWrite(stp_ALL,HIGH); //Trigger one step forward
    delay(1);
    digitalWrite(stp_ALL,LOW); //Pull step pin low so it can be triggered again
    delay(5);
  }
  resetDPins();
  delay(1000);
  client.publish("NMCU_RELAY/R5","off");
  Serial.println("Enter new option");
  Serial.println();
}

void Dose_C(){
  steps_C = C_dose*SPML_N;
  Serial.print("Moving forward ");Serial.print(steps_C);Serial.println(" steps.");
  Serial.print("Dosing: ");Serial.print(C_dose);Serial.println(" ml of nute C");
  digitalWrite(dir_ALL, LOW); //Pull direction pin low to move "forward"
  for(c= 1; c<steps_C; c++){
    digitalWrite(stp_ALL,HIGH); //Trigger one step forward
    delay(1);
    digitalWrite(stp_ALL,LOW); //Pull step pin low so it can be triggered again
    delay(5);
  }
  resetDPins();
  delay(1000);
  client.publish("NMCU_RELAY/R5","off");
  Serial.println("Enter new option");
  Serial.println();
}

void Dose_D(){
  steps_D = D_dose*SPML_D;
  Serial.print("Moving forward ");Serial.print(steps_D);Serial.println(" steps.");
  Serial.print("Dosing: ");Serial.print(D_dose);Serial.println(" ml of nute D");
  digitalWrite(dir_ALL, LOW); //Pull direction pin low to move "forward"
  for(d= 1; d<steps_D; d++){
    digitalWrite(stp_ALL,HIGH); //Trigger one step forward
    delay(1);
    digitalWrite(stp_ALL,LOW); //Pull step pin low so it can be triggered again
    delay(5);
  }
  resetDPins();
  delay(1000);
  client.publish("NMCU_RELAY/R5","off");
  Serial.println("Enter new option");
  Serial.println();
}

void resetDPins(){
  digitalWrite(stp_ALL, LOW);
  digitalWrite(EN_A, HIGH);
  digitalWrite(EN_B, HIGH);
  digitalWrite(EN_C, HIGH);
  digitalWrite(EN_D, HIGH);
}

bool inRange(int val, int minimum, int maximum) {
  return ((minimum < val) && (val < maximum));
}
