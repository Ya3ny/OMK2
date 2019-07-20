/*
-----------------------------------------------------------

Name: HomeLab Arduino Server
Description: HomeLab pH-meter user program
Version: 180522AS
Hardware: Arduino
WebHome: http://homelab.link/ph/meter/software/arduino/

Note: HomeLab_pH library is needed.

Copyright 2017-2018 Jivko Kalaydzhiev; HomeLab.link

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

------------------------------------------------------------

Note: Server error codes are listed at the bottom of this file.
*/

/*
      pH-Buffers:
  1 - pH =  4.00 Phthalate
  2 - pH =  7.00 Phosphate
  3 - pH = 10.01 Carbonate



  GPIO pins set selection

  One and only one GPIO-pins-set should be selected
  Uncomment the code lines for the selected set and
  comment the lines of all the other sets


  GPIO-pins-set 1
  Uno,Mega,BT,Ethernet,Leonardo,Pro,Duemilanove,
  M0,Zero,Due,Tian


const byte LED_pin = 13; // LED|button pin number
const byte W1_pin  = 12; // T-sensor (1-wire) pin number
const byte i2c_SCL = 11; // I2C SCL pin number
const byte i2c_SDA = 10; // I2C SDA pin number

/*

  GPIO-pins-set 2
  Nano,Mini,Micro

const byte LED_pin = 2; // LED|button pin number
const byte W1_pin  = 3; // T-sensor (1-wire) pin number
const byte i2c_SCL = 4; // I2C SCL pin number
const byte i2c_SDA = 5; // I2C SDA pin number
*/

/*
  GPIO-pins-set ESP8266
  ESP-12, Wemos-D1 mini
*/
const byte LED_pin = 13; // [D7] LED|button pin number         
const byte W1_pin  = 12; // [D6] T-sensor (1-wire) pin number
const byte i2c_SCL =  4; // [D2] I2C SCL pin number
const byte i2c_SDA =  5; // [D1] I2C SDA pin number



byte Tsensor_ID[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};


#include <HomeLab_pH.h>
HomeLab_pH HoLa(i2c_SCL, i2c_SDA, LED_pin, W1_pin, Tsensor_ID);

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

#define DHTTYPE DHT11   // DHT 11

const char* ssid = "C170512D5CFB";
const char* password = "9053393116";

// Change the variable to your Raspberry Pi IP address
const char* mqtt_server = "192.168.0.215";

WiFiClient NMCU_SENSE;
PubSubClient client(NMCU_SENSE);

const int DHTPin = 14; // [D5]

// Initialize DHT sensor.
DHT dht(DHTPin, DHTTYPE);

long now = millis();
long lastMeasure = 0;

const byte numReadings = 20;     //the number of sample times
byte ECsensorPin = A0;  //EC Meter analog output,pin on analog 1
unsigned int AnalogSampleInterval=15000;
unsigned int readings[numReadings];      // the readings from the analog input
byte InDeX = 0;                  // the index of the current reading
unsigned long AnalogValueTotal = 0;                  // the running total
unsigned int AnalogAverage = 0,averageVoltage=0;                // the average
unsigned long AnalogSampleTime,printTime,tempSampleTime;
float ECcurrent; 


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

int inByte;
float celsius = 25.0, Vph;
boolean PCB_check = false;

char swv[]={"180522AS"};

//
//--------------------------------------------------------
//
//  get Real Time and Date
//
 
void get_time_now(void)
{
  HoLa.RTC_min=0x00; 
  HoLa.RTC_hour=0x00; 
  HoLa.RTC_day=0x00; 
  HoLa.RTC_month=0x00; 
  HoLa.RTC_year=0x00; 
  
/*
 * 
 *  put your code to read RTC here
 * 
 *  and set proper values in BCD format to date/time variables
 */
 
// in BCD format

/*
  HoLa.RTC_min = 
  HoLa.RTC_hour=
  HoLa.RTC_day = 
  HoLa.RTC_mont=
  HoLa.RTC_year=
*/
}

//
//--------------------------------------------------------
//
//  get Button state
//

void Get_BTN(void)
{
  Serial.print("BTN: ");
  Serial.print(HoLa.get_button());
}

//--------------------------------------------------------
//
//  ADC measure whit change resolution - Vref and print
//

void Get_V(void)
{
  Serial.print("V:"); 
  if(HoLa.ADC_err) {Serial.print("ADC_err = "); Serial.println(HoLa.ADC_err); return;} 
  
  Vph=HoLa.get_voltage();

  if(Vph >= 0) Serial.print(" ");
  if(abs(Vph) < 1000) Serial.print(" ");
  if(abs(Vph) < 100) Serial.print(" ");
  if(abs(Vph) < 10) Serial.print(" ");
  Serial.print(Vph, 1);
  Serial.print(" [mV]");
}

//--------------------------------------------------------

void Single_shot(void)
{
float temp, volt, pH;

  Serial.print("V="); 
  if(HoLa.ADC_err) Serial.print(" ******");
  else 
    {
      HoLa.led_OFF(); delay(5);
      volt=HoLa.get_voltage();
      HoLa.led_ON();
      if(volt >= 0) Serial.print(" ");
      if(abs(volt) < 1000) Serial.print(" ");
      if(abs(volt) < 100) Serial.print(" ");
      if(abs(volt) < 10) Serial.print(" ");
      Serial.print( volt, 1);
    }
  Serial.print("[mV]  ");  

  Serial.print("T="); 
  if(HoLa.Tsens_check()) {temp=HoLa.get_temp(); Serial.print(" ");}
  else {temp=25.0; Serial.print("*");}              // with no sensor connected temperature is assumed to be 25.0
  if(temp<10) Serial.print(" ");
  Serial.print(temp ,1); Serial.print("[C]  "); 

  HoLa.led_OFF(); 
  Serial.print("pH="); 
  if(HoLa.Calib_err || HoLa.ADC_err) Serial.print("******");
  else
    {
      pH=HoLa.get_pH(volt, temp);
      if(HoLa.Tsens_err)  Serial.print("*");
      else                Serial.print(" ");
      if(pH>0 && pH<10)   Serial.print(" ");
      Serial.print( pH, 2);
    }
  Serial.print("  BTN="); Serial.println(HoLa.get_button());
}

void print_Buffers_pH(int b_txt)
{
float x;

  Serial.print("pH = ");
  x=get_Buffer_pH(b_txt, 25);
  if(x < 10) Serial.print(" ");
  Serial.print(x, 2);
}

//--------------------------------------------------------
//
//
void print_PCB_info(void)
{
float sloop, asym;
  
  Serial.println(); 
  Serial.println();
  
  Serial.print("ADC test            : "); 
  if(HoLa.ADC_err)  
    {
      Serial.print("failed;  ADC_err = "); 
      Serial.println(HoLa.ADC_err);
    } 
  else Serial.println("passed");
  
  Serial.print("T-sensor test       : "); 
  if(HoLa.Tsens_check())  Serial.println("passed");
  else                    
    {
      Serial.print("failed;  Tsens_err = "); 
      Serial.println(HoLa.Tsens_err);
    } 
    
  Serial.print("EEPROM test         : "); 
  if(HoLa.EPR_err)  
    {
      Serial.print("failed;  EPR_err = "); Serial.println(HoLa.EPR_err);
      Serial.println();
      Serial.println("No board data!"); 
    }
  else                                                
    {
      Serial.println("passed");
      Serial.print("Calib. data test    : "); 
      HoLa.calib_check();
      if(!HoLa.Calib_err) Serial.println("passed");
      else 
        {
          Serial.print("failed;  Calib_err = "); 
          Serial.println(HoLa.Calib_err); 
        } 
      
      Serial.println();
      Serial.print("Software version    : "); Serial.println(swv);
      Serial.print("Board revision"); spc(6); Serial.print(": ");
      Serial.print(HoLa.PCB_rev);
      Serial.print(".");
      Serial.print(HoLa.PCB_sub_rev);
      Serial.println();
      Serial.print("Board SN"); spc(12); Serial.print(": ");
      HoLa.print_PCB_ID(); 
      Serial.println();
    }
        
  if(HoLa.Tsens_check()) 
    {
     Serial.print("T-sensor ID         : ");
     HoLa.get_temp();
     HoLa.print_Tsens_ID();  Serial.println();   
     Serial.print("T-sensor resolution : ");
     switch (HoLa.Tsens_res)
       {
        case  9: {Serial.println("0.50 [C]"); break;} 
        case 10: {Serial.println("0.25 [C]"); break;}
        case 11: {Serial.println("0.12 [C]"); break;}
        case 12: {Serial.println("0.06 [C]"); break;}
        default:   {Serial.println("undefined!");}
       }
     }
     
  Serial.println();
  HoLa.calib_check();
  
  if(HoLa.Calib_err) { Serial.println("No valid calibration data!"); return; }

  sloop=1; asym=0;

  Serial.print("calibration point 1 : buffer No."); 
  Serial.print((int)HoLa.N_buf1); Serial.print("; ");  
  print_Buffers_name(HoLa.N_buf1); Serial.print("  "); 
  print_Buffers_pH(HoLa.N_buf1); Serial.println();
  spc(20); Serial.print(": T  = ");    
  Serial.print(HoLa.T_calib1, 1); Serial.println(" [C]"); 
  if( HoLa.calib_time_1 )
    {
      Serial.print("measured on         : ");  
      HoLa.print_calib_time(1);
      Serial.println();
    }
  Serial.println();
  Serial.print("calibration point 2 : buffer No."); 
  Serial.print((int)HoLa.N_buf2); Serial.print("; ");  
  print_Buffers_name(HoLa.N_buf2); Serial.print("  "); 
  print_Buffers_pH(HoLa.N_buf2); Serial.println();
  spc(20); Serial.print(": T  = ");      
  Serial.print(HoLa.T_calib2, 1); Serial.println(" [C]"); 
  if( HoLa.calib_time_2 )
    {
      Serial.print("measured on         : ");  
      HoLa.print_calib_time(2);
      Serial.println();
    }  
  Serial.println();
  sloop=(HoLa.V_buf2-HoLa.V_buf1)/(HoLa.pH_buf2-HoLa.pH_buf1)/HoLa.mV_pH;
  asym=(7-HoLa.pH_buf1)*sloop*HoLa.mV_pH+HoLa.V_buf1;
  if(sloop<0) sloop*=-1;

  Serial.print("slope"); spc(15); Serial.print(": ");
  if( sloop < 1) Serial.print(" ");
  Serial.print(sloop*100,1); Serial.println(" [%]");
  Serial.print("asymmetry"); spc(11); Serial.print(": ");
  if( abs(asym) < 10) Serial.print(" ");
  if(asym > 0 ) Serial.print("+");
  Serial.print(asym,1);  Serial.println(" [mV]"); 
}


                     
//--------------------------------------------------------

void spc   (byte x)                   { while(x) {Serial.write(" "); x--;}}
void spc_LF(byte x) { Serial.println(); while(x) {Serial.write(" "); x--;}}

//--------------------------------------------------------

float get_Buffer_pH(int bnum, int temp)
{
float ca, cb, cc, cd;
float tk, x;

  switch(bnum)
    {
      case 1 :
        {
          ca = 0;
          cb = 6.6146;
          cc = -1.8509;
          cd = 3.2721;
          break;
        }
      case 2 :
        {
// Comment (uncomment) the lines to select a set of pH temperature coefficients for buffer No.2
/* pH-buffer 7.00 Phosphate
*/

          ca = 1722.78;
          cb = -3.6787;
          cc = 1.6436;
          cd = 0;

/* pH-buffer 6.86 Phosphate

          ca = 3459.39;
          cb = -21.0574;
          cc = 7.3301;
          cd = -6.2266;
*/
          break;
        }
      case 3 :
        {
          ca = 2557.1;
          cb = -4.2846;
          cc = 1.9185;
          cd = 0;
          break;
        }
      default : return(0);
    }

  tk = temp - HoLa.abs_zero;
  x = ca/tk + cb + cc/100*tk + cd/100000*tk*tk;
  return x;
}

//--------------------------------------------------------

void print_Buffers_name(int b_txt)
{
  float x;

  if(b_txt<1 || b_txt>3) {Serial.print("no calibration");return;}
 
  switch(b_txt)
    {
      case 1 : {Serial.print("Phthalate"); break;}
      case 2 : {Serial.print("Phosphate"); break;}
      case 3 : {Serial.print("Carbonate"); break;}
      default:  break;
    }
}

//--------------------------------------------------------

void send_ERR(int err_number)
{
  if(err_number==0) {Serial.print("OK");}
  else {Serial.print("ERR "); Serial.print(err_number);}
  Serial.println(); 
}


void minus_get(void) { Serial.print(" - get "); }
void minus_set(void) { Serial.print(" - set "); }

void send_COM_text(void)
{
  Serial.print("#GI"); minus_get(); Serial.println("system info");
  Serial.print("#GR"); minus_get(); Serial.println("READY state");
  Serial.print("#Gt"); minus_get(); Serial.println("T-sensor resolution");
  Serial.print("#GT"); minus_get(); Serial.println("temperature");
  Serial.print("#GV"); minus_get(); Serial.println("voltage");
  Serial.print("#GP"); minus_get(); Serial.println("pH");
  Serial.print("#GB"); minus_get(); Serial.println("button state");
  Serial.print("#GS"); minus_get(); Serial.println("slope");
  Serial.print("#GA"); minus_get(); Serial.println("asymmetry");
  Serial.print("#GC"); minus_get(); Serial.println("calibration state");
  Serial.print("#GL"); minus_get(); Serial.println("measured data");
  Serial.print("#SPnx"); minus_set(); Serial.println("calibration point");
  spc(8); Serial.println("n=1 - point 1");
  spc(8); Serial.println("n=2 - point 2");
  spc(8); Serial.println("x=1 - pH  4 Phthalate");
  spc(8); Serial.println("x=2 - pH  7 Phosphate");
  spc(8); Serial.println("x=3 - pH 10 Carbonate");
  Serial.print("#Stx "); minus_set(); Serial.println("T-sensor resolution");
  spc(8); Serial.println("x=1 - 0.50  [C]");
  spc(8); Serial.println("x=2 - 0.25  [C]");
  spc(8); Serial.println("x=3 - 0.125 [C]");
  spc(8); Serial.println("x=4 - 0.06  [C]");
  Serial.print("#SLx "); minus_set(); Serial.println("LED state");
  spc(8); Serial.println("x=0 - OFF");
  spc(8); Serial.println("x=1 - ON");
  Serial.println();
}
//--------------------------------------------------------
//
//  
//

void callback(String topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  int pHb_1, pHb_2;
  float x1, x2, mV_pH_t;
  boolean tsr;

  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  if(topic=="sense/pH"){
    if(HoLa.EPR_err)        {send_EPR_ERR();    return;}
    if(HoLa.ADC_err)        {send_ADC_ERR();    return;}
    if(!HoLa.Tsens_check()) {send_Tsens_ERR();  return;}
    if(messageTemp=="measure") {
      measure_pH();
    }
    if(messageTemp=="SP11") {
      Serial.println("Calibrating SP1, pH 4 buffer\n");
      pHb_1=1;
    
      Vph=HoLa.get_voltage();
      celsius=HoLa.get_temp();
      mV_pH_t = HoLa.mV_pH/(25.0-HoLa.abs_zero)*(celsius-HoLa.abs_zero);
      x1=Vph*(25.0-HoLa.abs_zero)/(celsius-HoLa.abs_zero);
      x2=get_Buffer_pH(pHb_1, celsius);
      
      if((x1 < (7.0-x2)*mV_pH_t+40.0) && (x1 > (7.0-x2)*mV_pH_t-40.0))
      { 
        get_time_now(); 
        HoLa.set_calib_point(1, pHb_1, x2, x1, celsius);
        HoLa.calib_check(); 
        send_ERR(0); return;  
      }
      else {send_ERR(25); return;}
    }
        
    if(messageTemp=="SP12") {
      Serial.println("Calibrating SP1, pH 7 buffer\n");
      pHb_2=2;
  
      Vph=HoLa.get_voltage();
      celsius=HoLa.get_temp();
      mV_pH_t = HoLa.mV_pH/(25.0-HoLa.abs_zero)*(celsius-HoLa.abs_zero);
      x1=Vph*(25.0-HoLa.abs_zero)/(celsius-HoLa.abs_zero);
      x2=get_Buffer_pH(pHb_1, celsius);
      
      if((x1 < (7.0-x2)*mV_pH_t+40.0) && (x1 > (7.0-x2)*mV_pH_t-40.0))
      { 
        get_time_now(); 
        HoLa.set_calib_point(1, pHb_1, x2, x1, celsius);
        HoLa.calib_check(); 
        send_ERR(0); return;  
      }
      else {send_ERR(25); return;}
    }
    
    if(messageTemp=="SP21") {
      Serial.println("Calibrating SP2, pH 4 buffer\n");
      pHb_2=1;
      
      Vph=HoLa.get_voltage();
      celsius=HoLa.get_temp();
      mV_pH_t = HoLa.mV_pH/(25.0-HoLa.abs_zero)*(celsius-HoLa.abs_zero);
      x1=Vph*(25.0-HoLa.abs_zero)/(celsius-HoLa.abs_zero);
      x2=get_Buffer_pH(pHb_2, celsius); 
      if((x1 < (7.0-x2)*mV_pH_t+40.0) && (x1 > (7.0-x2)*mV_pH_t-40.0)) 
      { 
        get_time_now(); 
        HoLa.set_calib_point(2, pHb_2, x2, x1, celsius);
        HoLa.calib_check(); 
        send_ERR(0); return;  
      }
      else {send_ERR(25); return;}
      
    }
    
    if(messageTemp=="SP22") {
      Serial.println("Calibrating SP2, pH 7 buffer\n");
      pHb_2=2;
    
      Vph=HoLa.get_voltage();
      celsius=HoLa.get_temp();
      mV_pH_t = HoLa.mV_pH/(25.0-HoLa.abs_zero)*(celsius-HoLa.abs_zero);
      x1=Vph*(25.0-HoLa.abs_zero)/(celsius-HoLa.abs_zero);
      x2=get_Buffer_pH(pHb_2, celsius); 
      if((x1 < (7.0-x2)*mV_pH_t+40.0) && (x1 > (7.0-x2)*mV_pH_t-40.0)) 
      { 
        get_time_now(); 
        HoLa.set_calib_point(2, pHb_2, x2, x1, celsius);
        HoLa.calib_check(); 
        send_ERR(0);  return;  
      }
      else {send_ERR(25); return;}
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
    if (client.connect("NMCU_SENSE")) {
      Serial.println("connected");  
      // Subscribe or resubscribe to a topic
      // You can subscribe to more topics (to control more LEDs in this example)
      client.subscribe("sense/pH");
      client.subscribe("calibrate/EC");
      
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void measure_pH(){
  float x1, x2, mV_pH_t;
  
  static char pHTemp[7];
  dtostrf(x2,3,1,pHTemp);

    if(HoLa.EPR_err)        {send_EPR_ERR();    inByte=0; return;}   
    if(!HoLa.Tsens_check()) {send_Tsens_ERR();  inByte=0; return;}         
    if(HoLa.calib_check()) 
      {
        if(HoLa.ADC_err)    {send_ADC_ERR();    inByte=0;}
        Vph=HoLa.get_voltage();
        celsius=HoLa.get_temp();
        x1=Vph*(25.0-HoLa.abs_zero)/(celsius-HoLa.abs_zero);
        x2 = HoLa.pH_buf1+(HoLa.pH_buf2-HoLa.pH_buf1)*(x1-HoLa.V_buf1)/(HoLa.V_buf2-HoLa.V_buf1);
        Serial.print("pH: ");Serial.println(x2,2);Serial.print("mV: ");Serial.println(Vph,1);
      }
    else send_Calib_ERR(); 
    inByte=0;

    dtostrf(x2,3,1,pHTemp);

    client.publish("sense/pH",pHTemp);
}

void setup() {
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  
  int i=0;

  dht.begin();
  
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);


  HoLa.system_INIT(); // to be called before any other library function

  if(HoLa.ADC_err && HoLa.EPR_err) PCB_check=false;
  else                             PCB_check=true;

  while(Serial.available()) {inByte = Serial.read();}
  inByte=0;

  for (byte thisReading = 0; thisReading < numReadings; thisReading++)
  readings[thisReading] = 0;
  AnalogSampleTime=millis();
}


void loop() 
{
  if (!client.connected()) {
    reconnect();
  }
  if(!client.loop())
    client.connect("NMCU_SENSE");
  
  int pHb_1, pHb_2;
  float x1, x2, mV_pH_t;
  boolean tsr;

  float TempCoefficient=1.0+0.0185*(celsius-25.0);
  float CoefficientVoltage=(float)averageVoltage/TempCoefficient;   
    

  now = millis();
  // Publishes new sensor readings every 10 seconds
  if (now - lastMeasure > 10000) {
    lastMeasure = now;

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

    static char celsiusTemp[7];
    dtostrf(celsius, 2, 1, celsiusTemp);

    Serial.print("Humidity: ");
    Serial.print(h);
    
    Serial.print(" \t AIR [T]: ");
    Serial.print(t);
    
    Serial.print(" *C");
    Serial.print(" \t H20 [T]: ");
    Serial.print(celsiusTemp);
    Serial.println(" *C\n");
    
    measure_pH();
    client.publish("sense/AIR/T", temperatureTemp);
    client.publish("sense/AIR/H", humidityTemp);
    client.publish("sense/H2O/T", celsiusTemp);

/* EC */
  /*
    if(CoefficientVoltage<150)Serial.println("EC: no solution");   //25^C 1413us/cm<-->about 216mv  if the voltage(compensate)<150,that is <1ms/cm,out of the range
    else if(CoefficientVoltage>3300)Serial.println("Out of the range!");  //>20ms/cm,out of the range
    else
    { 
      if(CoefficientVoltage<=448)ECcurrent=6.84*CoefficientVoltage-64.32;   //1ms/cm<EC<=3ms/cm
      else if(CoefficientVoltage<=1457)ECcurrent=6.98*CoefficientVoltage-127;  //3ms/cm<EC<=10ms/cm
      else ECcurrent=5.3*CoefficientVoltage+2278;                           //10ms/cm<EC<20ms/cm
      ECcurrent/=1000;    //convert µS/cm to ms/cm
      Serial.print(ECcurrent,2);  //two decimal
      Serial.println("ms/cm");
    }
  }

  if(millis()-AnalogSampleTime>=AnalogSampleInterval)  
  {
    AnalogSampleTime=millis();
     // subtract the last reading:
    AnalogValueTotal = AnalogValueTotal - readings[InDeX];
    // read from the sensor:
    readings[InDeX] = analogRead(ECsensorPin);
    // add the reading to the total:
    AnalogValueTotal = AnalogValueTotal + readings[InDeX];
    // advance to the next position in the array:
    InDeX = InDeX + 1;
    // if we're at the end of the array...
    if (InDeX >= numReadings)
    // ...wrap around to the beginning:
    InDeX = 0;
    // calculate the average:
    AnalogAverage = AnalogValueTotal / numReadings;

    if(CoefficientVoltage<150)Serial.println("No solution!");   //25^C 1413us/cm<-->about 216mv  if the voltage(compensate)<150,that is <1ms/cm,out of the range
    else if(CoefficientVoltage>3300)Serial.println("Out of the range!");  //>20ms/cm,out of the range
    else { 
      if(CoefficientVoltage<=448)ECcurrent=6.84*CoefficientVoltage-64.32;   //1ms/cm<EC<=3ms/cm
      else if(CoefficientVoltage<=1457)ECcurrent=6.98*CoefficientVoltage-127;  //3ms/cm<EC<=10ms/cm
      else ECcurrent=5.3*CoefficientVoltage+2278;                           //10ms/cm<EC<20ms/cm
      ECcurrent/=1000;    //convert us/cm to ms/cm
      Serial.print(ECcurrent,2);  //two decimal
      Serial.println("ms/cm");
    }
*/
  }


    if (Serial.available()) inByte = Serial.read();
    if(inByte=='?') 
       {
        Serial.println("set of commands:");
        send_COM_text();  
        inByte=0; 
       }
    if(inByte!=0 && !PCB_check) {send_ERR(1); inByte=0;} 
         
Beg:
    if(inByte=='#') 
      {
        while(!Serial.available()) ; 
        inByte = Serial.read();
        if(inByte=='#') goto Beg;

//------------------------------------------------------------

        switch (inByte)
         {
          case 'G':                              // get mode
           {      
            while(!Serial.available()) ; 
            inByte = Serial.read();          
             
            if(inByte=='#') goto Beg;
            switch (inByte)
              {
                case 'I':                         // get info
                  {
                    print_PCB_info(); Serial.println(); inByte=0; break;  
                  }

                case 'R':                         // get Ready
                  {
                    if(HoLa.ADC_err)        {send_ADC_ERR();   inByte=0; break;}  
                    if(HoLa.EPR_err)        {send_EPR_ERR();   inByte=0; break;}   
                    if(!HoLa.Tsens_check()) {send_Tsens_ERR(); inByte=0; break;}
                    if(!HoLa.calib_check()) {send_Calib_ERR(); inByte=0; break;}
                    send_ERR(0); inByte=0;                      break;  
                  }

                case 't':                         // get T-sensor resolution
                  {
                    if(!HoLa.Tsens_check()) {send_Tsens_ERR(); inByte=0; break;}
                    HoLa.get_temp();
                    switch (HoLa.Tsens_res)
                      {
                        case  9: {Serial.println( 0.5, 2); break;}
                        case 10: {Serial.println(0.25, 2); break;}
                        case 11: {Serial.println(0.12, 2); break;}
                        case 12: {Serial.println(0.06, 2); break;}
                        default:   {send_ERR(10); break;}                    
                      }
                    inByte=0;
                    break;
                  }

                case 'T':                           // get Temperature
                  {
                    if(!HoLa.Tsens_check()) {send_Tsens_ERR(); inByte=0; break;}  
                    celsius=HoLa.get_temp(); Serial.println(celsius, 1);
                    inByte=0;
                    break;
                  }

                case 'V':                           // get Voltage
                  {
                    if(HoLa.EPR_err)  {send_EPR_ERR(); inByte=0; break;}   
                    if(!HoLa.ADC_err) {Vph=HoLa.get_voltage(); Serial.println(Vph, 1);}  
                    else send_ADC_ERR();                                       
                    inByte=0;
                    break;
                  }

                case 'P':                           // get pH
                  {
                    if(HoLa.EPR_err)        {send_EPR_ERR();    inByte=0; break;}   
                    if(!HoLa.Tsens_check()) {send_Tsens_ERR();  inByte=0; break;}         
                    if(HoLa.calib_check()) 
                      {
                        if(HoLa.ADC_err)    {send_ADC_ERR();    inByte=0; break;}
                        Vph=HoLa.get_voltage();
                        celsius=HoLa.get_temp();
                        x1=Vph*(25.0-HoLa.abs_zero)/(celsius-HoLa.abs_zero);
                        x2 = HoLa.pH_buf1 + (HoLa.pH_buf2-HoLa.pH_buf1)*(x1-HoLa.V_buf1)/(HoLa.V_buf2-HoLa.V_buf1);
                        Serial.println(x2, 2); 
                      }
                    else send_Calib_ERR(); 
                    inByte=0;
                    break;
                  }

                case 'B':                           // get Button state
                  {
                    Serial.println(HoLa.get_button()); inByte=0; break;
                  }

                case 'S':                           // get slope
                  {
                    if(HoLa.EPR_err)            {send_EPR_ERR(); inByte=0; break;}   
                    if(!HoLa.Calib_err)  
                      {
                        if(!HoLa.calib_check()) {send_Calib_ERR(); inByte=0; break;}     
                        x1=(HoLa.V_buf2-HoLa.V_buf1)/(HoLa.pH_buf2-HoLa.pH_buf1)/HoLa.mV_pH;
                        x2=(7-HoLa.pH_buf1)*x1*HoLa.mV_pH+HoLa.V_buf1;
                        if(x1<0) x1*=-1;
                        Serial.println(x1*100, 1);
                      }
                    else send_Calib_ERR();                
                    inByte=0; 
                    break;
                  }

                case 'A':                         // get asymmetry
                  {
                    if(HoLa.EPR_err)            {send_EPR_ERR(); inByte=0; break;}   
                    if(!HoLa.Calib_err) 
                      {
                        if(!HoLa.calib_check()) {send_Calib_ERR(); inByte=0; break;}  
                        x1=(HoLa.V_buf2-HoLa.V_buf1)/(HoLa.pH_buf2-HoLa.pH_buf1)/HoLa.mV_pH;
                        x2=(7-HoLa.pH_buf1)*x1*HoLa.mV_pH+HoLa.V_buf1;
                        Serial.println(x2, 1);
                      }
                    else send_Calib_ERR();  
                    inByte=0; 
                    break;
                  }

                case 'C':                         // get calib.
                  {                
                    if(HoLa.EPR_err)        {send_EPR_ERR();   inByte=0; break;}   
                    if(!HoLa.calib_check()) {send_Calib_ERR(); inByte=0; break;}  
                    send_ERR(0); inByte=0;  
                    break;
                  }

                case 'L':                         // get LINE - single shot
                  {
                    if(HoLa.EPR_err) {send_EPR_ERR(); inByte=0; break;}   
                    Single_shot(); inByte=0;  
                    break;
                  }

                default : 
                  {
                    send_ERR(33);   
                    inByte=0;  
                    break;
                  }
              }
           break;    
          }
          
//------------------------------------------------------------

          case 'S':                              // set mode
           {
            while(!Serial.available()) ; 
            inByte = Serial.read();          
            if(inByte=='#') goto Beg;
            switch (inByte)
              {
                case 'P':                         // set point X pHx, Vx and Tx
                  {
                    if(HoLa.EPR_err)        {send_EPR_ERR();    inByte=0; break;}   
                    if(HoLa.ADC_err)        {send_ADC_ERR();    inByte=0; break;}         
                    if(!HoLa.Tsens_check()) {send_Tsens_ERR();  inByte=0; break;}
                                           
                    while(!Serial.available()) ; 
                    inByte = Serial.read();          
                    if(inByte=='#') goto Beg;
                    
                    if(inByte=='1') 
                      {
                        while(!Serial.available()) ; 
                        inByte = Serial.read();          
                        if(inByte=='#') goto Beg;
                        if(inByte=='1') { pHb_1=1; goto LP_1;}
                        if(inByte=='2') { pHb_1=2; goto LP_1;}
                        if(inByte=='3') { pHb_1=3; goto LP_1;}
                        send_ERR(24); inByte=0; break;   
LP_1:
                        Vph=HoLa.get_voltage();
                        celsius=HoLa.get_temp();
                        mV_pH_t = HoLa.mV_pH/(25.0-HoLa.abs_zero)*(celsius-HoLa.abs_zero);
                        x1=Vph*(25.0-HoLa.abs_zero)/(celsius-HoLa.abs_zero);
                        x2=get_Buffer_pH(pHb_1, celsius); 
                        if((x1 < (7.0-x2)*mV_pH_t+40.0) && (x1 > (7.0-x2)*mV_pH_t-40.0)) 
                            { 
                              get_time_now(); 
                              HoLa.set_calib_point(1, pHb_1, x2, x1, celsius);
                              HoLa.calib_check(); 
                              send_ERR(0); inByte=0; break;  
                            }
                        else {send_ERR(25); inByte=0; break;}
                      }

                    if(inByte=='2') 
                      {
                        while(!Serial.available()) ; 
                        inByte = Serial.read();          
                        if(inByte=='#') goto Beg;
                        
                        if(inByte=='1') { pHb_2=1; goto LP_2;}
                        if(inByte=='2') { pHb_2=2; goto LP_2;}
                        if(inByte=='3') { pHb_2=3; goto LP_2;}
                        send_ERR(24); inByte=0; break;   
LP_2:
                        Vph=HoLa.get_voltage();
                        celsius=HoLa.get_temp();
                        mV_pH_t = HoLa.mV_pH/(25.0-HoLa.abs_zero)*(celsius-HoLa.abs_zero);
                        x1=Vph*(25.0-HoLa.abs_zero)/(celsius-HoLa.abs_zero);
                        x2=get_Buffer_pH(pHb_2, celsius); 
                        if((x1 < (7.0-x2)*mV_pH_t+40.0) && (x1 > (7.0-x2)*mV_pH_t-40.0)) 
                            { 
                              get_time_now(); 
                              HoLa.set_calib_point(2, pHb_2, x2, x1, celsius);
                              HoLa.calib_check(); 
                              send_ERR(0);  inByte=0; break;  
                            }
                        else {send_ERR(25); inByte=0; break;}
                      }
                    send_ERR(26); 
                    inByte=0; 
                    break;  
                  }
                  
                case 't':                         // set T-sensor resolution
                  {
                    if(!HoLa.Tsens_check()) {send_Tsens_ERR(); inByte=0; break;}  
                    while(!Serial.available()) ; 
                    inByte = Serial.read();          
                    if(inByte=='#') goto Beg;
                    switch (inByte)
                        {
                           case '1': {tsr=HoLa.set_Tsens_res( 9); break;}
                           case '2': {tsr=HoLa.set_Tsens_res(10); break;}
                           case '3': {tsr=HoLa.set_Tsens_res(11); break;}
                           case '4': {tsr=HoLa.set_Tsens_res(12); break;}
                           default:  {send_ERR(10); inByte=0; break;}  
                        } 

                    if(!tsr) {send_ERR(27); inByte=0; break;}  
                    send_ERR(0); inByte=0; 
                    break;
                  }
                  
                case 'L':                         // set LED
                  {
                    while(!Serial.available()) ; 
                    inByte = Serial.read();          
                    if(inByte=='#') goto Beg;

                    if(inByte=='1') {HoLa.led_ON();  inByte=0;}
                    if(inByte=='0') {HoLa.led_OFF(); inByte=0;} 
                    if(inByte==0) send_ERR( 0);
                    else          send_ERR(28); 
                    inByte=0; 
                    break;
                  }

                default : 
                  {
                    send_ERR(33);  
                    inByte=0;
                    break;
                  }
              }
           break;              
          }
          
          default: 
            {
              send_ERR(33); 
              inByte=0;
              break;
            }
        }        
     }

}         

void send_ADC_ERR(void)
{
  if(HoLa.ADC_err==1) send_ERR(2); 
  if(HoLa.ADC_err==2) send_ERR(3); 
}
void send_EPR_ERR(void)
{
  if(HoLa.EPR_err==1) send_ERR(4); 
  if(HoLa.EPR_err==2) send_ERR(5); 
}
void send_Tsens_ERR(void)
{
  if(HoLa.Tsens_err==1) send_ERR(6); 
  if(HoLa.Tsens_err==2) send_ERR(7); 
  if(HoLa.Tsens_err==3) send_ERR(8); 
  if(HoLa.Tsens_err==4) send_ERR(9); 
}
void send_Calib_ERR(void)
{
  if(HoLa.Calib_err==1) send_ERR(11); 
  if(HoLa.Calib_err==2) send_ERR(12); 
  if(HoLa.Calib_err==3) send_ERR(13); 
  if(HoLa.Calib_err==4) send_ERR(14); 
  if(HoLa.Calib_err==5) send_ERR(15); 
  if(HoLa.Calib_err==6) send_ERR(16); 
  if(HoLa.Calib_err==7) send_ERR(17); 
  if(HoLa.Calib_err==8) send_ERR(18); 
  if(HoLa.Calib_err==9) send_ERR(19); 
  if(HoLa.Calib_err==10) send_ERR(20); 
  if(HoLa.Calib_err==11) send_ERR(21); 
  if(HoLa.Calib_err==12) send_ERR(22); 
  if(HoLa.Calib_err==13) send_ERR(23); 
}

/*

Error codes description

 ERR1     Bad connection. (Check the assigned GPIO pin numbers or the cable.)  
 ERR2     The ADC-chip does not respond.  
 ERR3     The ADC-chip does not work properly.  
 ERR4     No EEPROM device found.  
 ERR5     Invalid EEPROM-chip record.  
 ERR6     No 1-wire device found.  
 ERR7     Temperature sensor bad CRC response (probable presence of multiple 1-wire devices).  
 ERR8     The temperature sensor ID does not match the user-set ID.  
 ERR9     The temperature sensor is not the HomeLab type sensor (DS18B20).  
 ERR10    The temperature sensor resolution is undefined.  
 ERR11    No calibration data is available.  
 ERR12    Invalid buffer number for calibration point 1 (must be 1,2 or 3).  
 ERR13    Invalid buffer number for calibration point 2 (must be 1,2 or 3).  
 ERR14    The pH of the calibration buffers is too close (difference must be > 1.0 pH).  
 ERR15    The buffer pH for calibration point 1 is out of the range 0 - 14 pH.  
 ERR16    The buffer pH for calibration point 2 is out of the range 0 - 14 pH.  
 ERR17    The voltage for calibration point 1 is out of range (theoretical value +-40 mV)*.  
 ERR18    The voltage for calibration point 2 is out of range (theoretical value +-40 mV)*.  
 ERR19    The temperature for calibration point 1 is out of the range 0Â°C - 100Â°C.  
 ERR20    The temperature for calibration point 2 is out of the range 0Â°C - 100Â°C.  
 ERR21    Invalid time and/or date for calibration point 1.  
 ERR22    Invalid time and/or date for calibration point 2.  
 ERR23    The time interval between the calibration points is longer than 50 min.  
 ERR24    Invalid buffer number.  
 ERR25    A parameter is out of limits.  
 ERR26    Invalid calibration point number.  
 ERR27    Can not set the temperature resolution.  
 ERR28    Invalid LED setting.  
 ERR33    Invalid request.  

*Note: the theoretical voltage value for a buffer at 25 C is:
59.16 mV * ((buffer pH at 25 C) - 7)

*/




