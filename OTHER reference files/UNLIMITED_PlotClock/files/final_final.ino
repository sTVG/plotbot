//MIx_torsdag_1
//
//WiFi.begin(ssid, password);

//to

//WiFi.begin(“device");


//Gruppe 4 - ULIMITED - kode der virker !!
// inkludere bibloteker 
#include <ESP8266WiFi.h>
#include <Servo.h>
#include "PubSubClient.h" // https://github.com/knolleary/pubsubclient/releases/tag/v2.3
#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson/releases/tag/v5.0.7
#include <Time.h>
#include <TimeLib.h>

//-------- Customise these values ----------- // INternet :D
const char* ssid = "Amalies iPhone";//"Elisabeths iPhone";  //"Elisabeths iPhone"; /
const char* password = "k8e6vr0q4pd62"; //"elisa1804"; // //"elisa1804"; //

#define ORG "dfzkww"
#define DEVICE_TYPE "ESP-DTU"
#define DEVICE_ID "group8"
#define TOKEN "zhLnvQYnLuvYF(MnLm"

//---------------------------------------------

// Definere funktioner 
void handleCommand(JsonObject& root);
void callback(char* topic, byte* payload, unsigned int payloadLength);
void wifiConnect();
void mqttConnect();
void subscribeTo(const char* topic);
void publishTo(const char* topic, const char* payload);
void plot(String mydata);
void drawTo(double pX, double pY);
void lift(char lift);
void number(float bx, float by, int num, float scale);
void bogenUZS(float bx, float by, float radius, int start, int ende, float sqee);
void bogenGZS(float bx, float by, float radius, int start, int ende, float sqee);
void set_XY(double Tx, double Ty) ;
void drej();
void turns(float rotations);
void phaseSelect(int phase);

//-------------------------------------- node-red ------------------------------------------

char server[] = ORG ".messaging.internetofthings.ibmcloud.com";
char authMethod[] = "use-token-auth";
char token[] = TOKEN;
char clientId[] = "d:" ORG ":" DEVICE_TYPE ":" DEVICE_ID;
const char commandTopic[] = "iot-2/cmd/blink/fmt/json";         //TOPIC !!!   //Skal der laves flere topics? fx en til vejr og en til pollen ?
const char publishTopic[] = "iot-2/evt/status/fmt/json";

WiFiClient wifiClient; //Create wifi client object
PubSubClient client(server, 1883, callback, wifiClient); //Create a PubSub MQTT Client

int publishInterval = 5000; //5 seconds interval between status updates to status topic
long lastPublishMillis; //Interval counter
//--------------------------------------------- plot clock --------------------------------------------
// Til at kaliberer: 
//#define CALIBRATION

// servofaktor start 640
#define SERVOFAKTOR 600
//Når taler blive mindre bevæger servo sig i færre grader og omvendt. 

// nul position på servo motorer (letter at rette manuelt) 
// left start 1900 right start 984
#define SERVOLEFTNULL 2100
#define SERVORIGHTNULL 960
// jo mindre værdi roterer den med uret og omvendt

// MCU Pins 
#define SERVOPINLIFT  D2
#define SERVOPINLEFT  D3
#define SERVOPINRIGHT D4

// lift positions of lifting servo
// start: lift0 1080 lift1 925 lift2 725
#define LIFT0 1900 // on drawing surface
#define LIFT1 1300  // between numbers
#define LIFT2 1300  // going towards sweeper
// lille tal løfter, stort tal sænker højde

// speed of liftimg arm, higher is slower
#define LIFTSPEED 2000

// length of arms
#define L1 35  //35
#define L2 55.1   // 55.1
#define L3 13.2    // 13.2

// origin points of left and right servo 
#define O1X 22
#define O1Y -25
#define O2X 47
#define O2Y -25

int servoLift = 1500;

Servo servo1;  // 
Servo servo2;  // 
Servo servo3;  // 

//Går tilbage til start position
volatile double lastX = 75;
volatile double lastY = 47.5;

int val = 0;
int last_min = 0;
String(myData);

//---------------------------------------------------- Servo-----------------------------------------------------------------
int valu;   //sætter val til en variabel
// define the stepper driver pins
#define IN1  D5

#define IN2  D6
#define IN3  D7
#define IN4  D8
// define how many cycles to a full rotation
#define CYCLES_PER_ROTATION 512
//Bånd definitioner
float pi = 3.14159;
int d = 12.5;
int x = 20;
double t = 0.5;
float M;
float nextM;
float nextOmk;
float Omk;
float bonusOmk;
float startOmk = pi * d;

//----------------------------------------------------- Void setup -----------------------------------------------
void setup() {
  // Opsætter node MCU 
  Serial.begin(115200);
  wifiConnect(); //Establish wifi-connection
  Serial.begin(115200); Serial.println(); //Initialize Serial Monitor for verbose trouble shooting
  mqttConnect(); //Connect to MQTT-Broker once wifi is up
  subscribeTo(commandTopic); //Subscribe to the command-topic once connected to the MQTT Broker

// Start position. 
  drawTo(72.2, 47);
  lift(2);
  servo1.attach(SERVOPINLIFT);  //  lifting servo
  servo2.attach(SERVOPINLEFT);  //  left servo
  servo3.attach(SERVOPINRIGHT);  //  right servo
  delay(1000);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  
}
//-------------------------------------------------------------Void loop----------------------------------------------------------
void loop() {
  
  #ifdef CALIBRATION

  // Servohorns will have 90° between movements, parallel to x and y axis
  drawTo(-3, 29.2);
  delay(500);
  drawTo(74.1, 28);
  delay(500);
  
  #else 
    if (!client.loop()) { //Make sure we are connected to the MQTT Broker
    mqttConnect();
    
    // Start position. 
  drawTo(72.2, 47);
  lift(2);
  servo1.attach(SERVOPINLIFT);  //  lifting servo
  servo2.attach(SERVOPINLEFT);  //  left servo
  servo3.attach(SERVOPINRIGHT);  // right servo
  delay(1000);

    Serial.print("Genopretter forbindelsen...");
    subscribeTo(commandTopic);
    //callback();
    }
   #endif

   if (millis() - lastPublishMillis > publishInterval) { //Check if interval and publish to status topic
    String payload = "{\"d\":{\"counter\":";
    payload += millis() / 1000;
    payload += "}}";
    publishTo(publishTopic, payload.c_str());
    lastPublishMillis = millis(); } 
  
} // Loop slut 

void handleCommand(JsonObject& root) {
  // you will have to write your code here. The code that makes awesome stuff from the Cloud commands your send your device.

  //Variable:
//Pollen
  int  birk = root["pollen"]["birk"];   //birk tal
  int  grass = root["pollen"]["grass"]; //
//Vejret
  String  vejr = root["weather"]["weatherin1hr"];
  int  temp = root["weather"]["temperaturein1hr"];
//Tid
  int tim = (int)root["dateTime"]["hour"];
  int minu = (int)root["dateTime"]["minutes"];
//Tweets
  String tweet = root["tweet"];
//Bonus
  int tid_BH =root["TimeStamp"]["hour"];
  int tid_BM =root["TimeStamp"]["minutes"];
  String BusNr =root["BusInfo"]["Bus nr"];

//----------------------------------------- Input fra node-red-----------------------------------------------------------------
//Print Tweet 
  //    
  if (tweet!=0){
  Serial.print("Tweet:"),Serial.println(tweet);
  Serial.println("");
  String(myData) = (tweet);
  plot(myData);
  }
  //Print pollen
  else if (birk!=0) {
  Serial.print("Pollen: "),Serial.print("B: "),Serial.print(birk),Serial.print("G:"),Serial.println(grass);
  Serial.println("");
  String(myData) = String("POLLEN:") + String('B')+String(":") +String(birk) +String('G')+String(grass);
  plot(myData);
  }
  //Print vejret:
  else if (vejr!=0) {
  Serial.print("T: "),Serial.print(temp),Serial.print("C"),Serial.print(vejr);
  Serial.println("");
  String(myData) = String("TEMP:")+String(temp)+String("C")+String(vejr);
  plot(myData);
  }
  //Print klokken:
  else if (tim!=0) {
  Serial.print("Klokken er nu: "),Serial.print(tim),Serial.print(":"),Serial.println(minu);
  Serial.println("");
  String(myData) = String("KL") +String(tim)+String(":")+String(minu);
  plot(myData);
  }
  //Print Bustider:
  else if(tid_BH!=0){
  Serial.print("Næste bus fra DTU: "),Serial.print(BusNr),Serial.print("Kl."),Serial.print(tid_BH),Serial.print(":"),Serial.println(tid_BM);
  Serial.println(""); 
  String(myData) = String("BUS FRA DTU")+String(BusNr)+String("KL")+String(tid_BH)+String(":")+String(tid_BM);
  plot(myData); 
    }
  else {
    Serial.print("Not working");
    }
  //}
}

//---------------------------------------------------------------plot-clock funktioner--------------------------------------------
void plot(String myData) {
  
  int data_len = myData.length()+1;
  int i;
  char char_array[data_len];
  myData.toCharArray(char_array, data_len);
  float XS = 20;
  float YS = 20;
  double big_len = 1;
  double short_len = 0.5;
  int paper_left = 1;
  
// Tjekker om servo
   if (!servo1.attached()) servo1.attach(SERVOPINLIFT);
   if (!servo2.attached()) servo2.attach(SERVOPINLEFT);
   if (!servo3.attached()) servo3.attach(SERVOPINRIGHT);

//Tjekker om der er bon rulle
  valu = digitalRead(D1); // kalder digital read af pin 9 for val 
  if (valu == HIGH) {
    Serial.println("Ingen rulle");   //Hvis der ikke er noget i sensoren, vil val være HIGH og den vil kalde fejl (skal også stoppe steppermotor!)
      //delay(2000);
      
      //Komando mangler så draw ikke virker !!
    }
    else if (valu != HIGH) {
    Serial.println("Nu kan vi skrive :)");   //Hvis der er noget i sensoren, vil val være LOW og bonrullen vil køre videre
    drej();       
    //delay(7000);
    
  //Tegne loop:
   for (i=0; i < data_len -1; i++) {
    Serial.print("in loop");
   
    if (myData[i] == '0') {
      number(XS,YS,'0',1.2); 
      drej();
      delay(500);
    }
    else if (myData[i] == '1') {
      number(XS, YS, '1', 1.2);
      drej();
      delay(500);
    }
    else if (myData[i] == '2') {
      number(XS,YS,'2',1.2);
      drej();
      delay(500);
      Serial.print("INDE");
    }
    else if (myData[i] == '3') {
      number(XS,YS,'3',1.2);
      drej();
      delay(500);
    }
    else if (myData[i] == '4') {
      number(XS,YS,'4',1.2);
      drej();
      delay(500);
    }
    else if (myData[i] == '5') {
      number(XS,YS,'5',1.2);
      drej();
      delay(500);
    }
    else if (myData[i] == '6') {
      number(XS,YS,'6',1.2);
      drej();
      delay(500);
    }
    else if (myData[i] == '7') {
      number(XS,YS,'7',1.2);
      drej();
      delay(500);
    }
    else if (myData[i] == '8') {
      number(XS,YS,'8',1.2);
      drej();
      delay(500);
    }
    else if (myData[i] == '9') {
      number(XS,YS,'9',1.2);
      drej();
      delay(500);
    }
    else if (myData[i] == '11') {
      number(XS,YS,'11',1.2);
      drej();
      delay(500);
    }
    else if (myData[i] == 'A') {
      number(XS,YS,'A',1.2);
      drej();
      delay(500);
    }
    else if (myData[i] == 'B') {
      number(XS,YS,'B',1.2);
      drej();
      delay(500);
    }
    else if (myData[i] == 'C') {
      number(XS,YS,'C',1.2);
      drej();
      delay(500);
    }
    else if (myData[i] == 'D') {
      number(XS,YS,'D',1.2);
      drej();
      delay(500);
    }
    else if (myData[i] == 'E') {
      number(XS,YS,'E',1.2);
      drej();
      delay(500);
    }
    else if (myData[i] == 'F') {
      number(XS,YS,'F',1.2);
      drej();
      delay(500);
    }
    else if (myData[i] == 'G') {
      number(XS,YS,'G',1.2);
      drej();
      delay(500);
    }
    else if (myData[i] == 'H') {
      number(XS,YS,'H',1.2);
      drej();
      delay(500);
    }
    else if (myData[i] == 'I') {
      number(XS,YS,'I',1.2);
      drej();
      delay(500);
    }
    else if (myData[i] == 'J') {
      number(XS,YS,'J',1.2);
      drej();
      delay(500);
    }
    else if (myData[i] == 'K') {
      number(XS,YS,'K',1.2);
      drej();
      delay(500);
    }
    else if (myData[i] == 'L') {
      number(XS,YS,'L',1.2);
      drej();
      delay(500);
    }
    else if (myData[i] == 'M') {
      number(XS,YS,'M',1.2);
      drej();
      delay(500);
    }
    else if (myData[i] == 'N') {
      number(XS,YS,'N',1.2);
      drej();
      delay(500);
    }
    else if (myData[i] == 'O') {
      number(XS,YS,'O',1.2);
      drej();
      delay(500);
    }
    else if (myData[i] == 'P') {
      number(XS,YS,'P',1.2);
      drej();
      delay(500);
    }
    else if (myData[i] == 'Q') {
      number(XS,YS,'Q',1.2);
      drej();
      delay(500);
    }
    else if (myData[i] == 'R') {
      number(XS,YS,'R',1.2);
      drej();
      delay(500);
    }
    else if (myData[i] == 'S') {
      number(XS,YS,'S',1.2);
      drej();
      delay(500);
    }
    else if (myData[i] == 'T') {
      number(XS,YS,'T',1.2);
      drej();
      delay(500);
    }
    else if (myData[i] == 'U') {
      number(XS,YS,'U',1.2);
      drej();
      delay(500);
    }
    else if (myData[i] == 'V') {
      number(XS,YS,'V',1.2);
      delay(500);
    }
    else if (myData[i] == 'W') {
      number(XS,YS,'W',1.2);
      drej();
      delay(500);
    }
    else if (myData[i] == 'X') {
      number(XS,YS,'X',1.2);
      drej();
      delay(500);
    }
    else if (myData[i] == 'Y') {
      number(XS,YS,'Y',1.2);
      drej();
      delay(500);
    }
    else if (myData[i] == 'Z') {
      number(XS,YS,'Z',1.2);
      drej();
      delay(500);
    }
    else if (myData[i] == ':') {
      number(XS,YS,':',1.2);
      drej();
      delay(500);
    }
    
    else if (myData[i] == 'SOL') {
      number(XS,YS,'SOL',1.2);
      drej();
      delay(500);
    }
    else if (myData[i] == 'sky') {
      number(XS,YS,'sky',1.2);
      drej();
      delay(500);
    }
    else if (myData[i] == 'regnsky') {
      number(XS,YS,'regnsky',1.2);
      drej();
      delay(500);
    } 
   }
//Detach servos 
  servo1.detach();
  servo2.detach();
  servo3.detach();  
}
}
void number(float bx, float by, int num, float scale) {

  switch (num) {

  case '0':
    drawTo(bx + 12 * scale, by + 6 * scale);
    lift(0);
    bogenGZS(bx + 7 * scale, by + 10 * scale, 10 * scale, -0.8, 6.7, 0.5);
    lift(1);
    break;
  case '1':

    drawTo(bx + 3 * scale, by + 15 * scale);
    lift(0);
    drawTo(bx + 10 * scale, by + 20 * scale);
    drawTo(bx + 10 * scale, by + 0 * scale);
    lift(1);
    break;
  case '2':
    drawTo(bx + 2 * scale, by + 12 * scale);
    lift(0);
    bogenUZS(bx + 8 * scale, by + 14 * scale, 6 * scale, 3, -0.8, 1);
    drawTo(bx + 1 * scale, by + 0 * scale);
    drawTo(bx + 12 * scale, by + 0 * scale);
    lift(1);
    break;
  case '3':
    drawTo(bx + 2 * scale, by + 17 * scale);
    lift(0);
    bogenUZS(bx + 5 * scale, by + 15 * scale, 5 * scale, 3, -2, 1);
    bogenUZS(bx + 5 * scale, by + 5 * scale, 5 * scale, 1.57, -3, 1);
    lift(1);
    break;
  case '4':
    drawTo(bx + 10 * scale, by + 0 * scale);
    lift(0);
    drawTo(bx + 10 * scale, by + 20 * scale);
    drawTo(bx + 2 * scale, by + 6 * scale);
    drawTo(bx + 12 * scale, by + 6 * scale);
    lift(1);
    break;
  case '5':
    drawTo(bx + 2 * scale, by + 5 * scale);
    lift(0);
    bogenGZS(bx + 5 * scale, by + 6 * scale, 6 * scale, -2.5, 2, 1);
    drawTo(bx + 5 * scale, by + 20 * scale);
    drawTo(bx + 12 * scale, by + 20 * scale);
    lift(1);
    break;
  case '6':
    drawTo(bx + 2 * scale, by + 10 * scale);
    lift(0);
    bogenUZS(bx + 7 * scale, by + 6 * scale, 6 * scale, 2, -4.4, 1);
    drawTo(bx + 11 * scale, by + 20 * scale);
    lift(1);
    break;
  case '7':
    drawTo(bx + 2 * scale, by + 20 * scale);
    lift(0);
    drawTo(bx + 12 * scale, by + 20 * scale);
    drawTo(bx + 2 * scale, by + 0);
    lift(1);
    break;
  case '8':
    drawTo(bx + 5 * scale, by + 10 * scale);
    lift(0);
    bogenUZS(bx + 5 * scale, by + 15 * scale, 5 * scale, 4.7, -1.6, 1);
    bogenGZS(bx + 5 * scale, by + 5 * scale, 5 * scale, -4.7, 2, 1);
    lift(1);
    break;

  case '9':
    drawTo(bx + 9 * scale, by + 11 * scale);
    lift(0);
    bogenUZS(bx + 7 * scale, by + 15 * scale, 5 * scale, 4, -0.5, 1);
    drawTo(bx + 5 * scale, by + 0);
    lift(1);
    break;
  case ':':
    drawTo(bx + 5 * scale, by + 15 * scale);
    lift(0);
    bogenGZS(bx + 5 * scale, by + 15 * scale, 0.1 * scale, 1, -1, 1);
    delay(10);
    lift(1);
    drawTo(bx + 5 * scale, by + 5 * scale);
    lift(0);
    bogenGZS(bx + 5 * scale, by + 5 * scale, 0.1 * scale, 1, -1, 1);
    delay(10);
    lift(1);
    break;
  case 'A':
    drawTo(bx+0*scale,by+0*scale);    
    lift(0);                           
    drawTo(bx+6*scale,by+20*scale);  
    drawTo(bx+12*scale,by+0*scale);
    lift(1);      
    drawTo(bx+3*scale,by+10*scale);
    lift(0);      
    drawTo(bx+9*scale,by+10*scale);
    lift(1);
    break;

  case 'B':
    drawTo(bx+0*scale,by+0*scale);   
    lift(0);                      
    drawTo(bx+0*scale,by+20*scale);
    drawTo(bx+9*scale,by+20*scale);
    drawTo(bx+12*scale,by+17*scale);
    drawTo(bx+12*scale,by+13*scale);
    drawTo(bx+9*scale,by+10*scale);
    drawTo(bx+12*scale,by+7*scale);
    drawTo(bx+12*scale,by+3*scale);
    drawTo(bx+9*scale,by+0*scale);
    drawTo(bx+0*scale,by+0*scale);
    lift(1);
    drawTo(bx+0*scale,by+10*scale);
    lift(0);
    drawTo(bx+9*scale,by+10*scale);
    lift(1);        
    break;

  case 'C':
    drawTo(bx+12*scale,by+20*scale);  
    lift(0);                      
    drawTo(bx+5*scale,by+20*scale); 
    drawTo(bx+0*scale,by+15*scale); 
    drawTo(bx+0*scale,by+5*scale);
    drawTo(bx+5*scale,by+0*scale);
    drawTo(bx+12*scale,by+0*scale);
    lift(1); 
    break;

  case 'D':
    drawTo(bx+0*scale,by+0*scale);    
    lift(0);                      
    drawTo(bx+0*scale,by+20*scale);
    drawTo(bx+7*scale,by+20*scale);
    drawTo(bx+12*scale,by+15*scale);
    drawTo(bx+12*scale,by+5*scale);
    drawTo(bx+7*scale,by+0*scale);
    drawTo(bx+0*scale,by+0*scale);
    lift(1);
    break;

  case 'E':
    drawTo(bx+12*scale,by+20*scale);
    lift(0);   
    drawTo(bx+0*scale,by+20*scale); 
    drawTo(bx+0*scale,by+0*scale);
    drawTo(bx+12*scale,by+0*scale);
    lift(1);    
    drawTo(bx+0*scale,by+10*scale);
    lift(0);
    drawTo(bx+8*scale,by+10*scale);
    lift(1);
    break; 

  case 'F':
    drawTo(bx+0*scale,by+0*scale);    
    lift(0);                      
    drawTo(bx+0*scale,by+20*scale); 
    drawTo(bx+12*scale,by+20*scale);
    lift(1);
    drawTo(bx+0*scale,by+10*scale);
    lift(0);
    drawTo(bx+8*scale,by+10*scale); 
    lift(1);
    break; 

  case 'G':
    drawTo(bx+12*scale,by+20*scale); 
    lift(0);
    drawTo(bx+5*scale,by+20*scale); 
    drawTo(bx+0*scale,by+15*scale);
    drawTo(bx+0*scale,by+5*scale); 
    drawTo(bx+5*scale,by+0*scale); 
    drawTo(bx+12*scale,by+0*scale);
    drawTo(bx+12*scale,by+10*scale);
    drawTo(bx+5*scale,by+10*scale);
    lift(1);
    break; 

  case 'H':
    drawTo(bx+0*scale,by+0*scale);    
    lift(0);                      
    drawTo(bx+0*scale,by+20*scale);
    lift(1);
    drawTo(bx+0*scale,by+10*scale);
    lift(0);
    drawTo(bx+12*scale,by+10*scale);
    lift(1);
    drawTo(bx+12*scale,by+20*scale);
    lift(0);
    drawTo(bx+12*scale,by+0*scale);
    lift(1);
    break;

  case 'I':
    drawTo(bx+3*scale,by+20*scale);
    lift(0);
    drawTo(bx+9*scale,by+20*scale);
    lift(1);
    drawTo(bx+6*scale,by+20*scale);
    lift(0);
    drawTo(bx+6*scale,by+0*scale);
    lift(1);
    drawTo(bx+3*scale,by+0*scale);
    lift(0);
    drawTo(bx+9*scale,by+0*scale);
    lift(1);
    break;

  case 'J':
    drawTo(bx+0*scale,by+20*scale);
    lift(0);
    drawTo(bx+12*scale,by+20*scale);
    drawTo(bx+12*scale,by+5*scale);
    drawTo(bx+9*scale,by+0*scale);
    drawTo(bx+2*scale,by+0*scale);
    drawTo(bx+0*scale,by+5*scale);
    drawTo(bx+0*scale,by+10*scale);
    lift(1);
    break; 

  case 'K':
    drawTo(bx+0*scale,by+0*scale);  
    lift(0);      
    drawTo(bx+0*scale,by+20*scale);
    lift(1);
    drawTo(bx+0*scale,by+10*scale);
    lift(0);
    drawTo(bx+12*scale,by+20*scale);
    lift(1);
    drawTo(bx+0*scale,by+10*scale);
    lift(0);
    drawTo(bx+12*scale,by+0*scale);
    lift(1);
    break; 

  case 'L':
    drawTo(bx+0*scale,by+20*scale);
    lift(0);      
    drawTo(bx+0*scale,by+0*scale);
    drawTo(bx+12*scale,by+0*scale);
    lift(1);
    break;

  case 'M':
    drawTo(bx+0*scale,by+0*scale);    
    lift(0);
    drawTo(bx+0*scale,by+20*scale);
    drawTo(bx+6*scale,by+10*scale);
    drawTo(bx+12*scale,by+20*scale);
    drawTo(bx+12*scale,by+0*scale);
    lift(1);
    break;

  case 'N':
    drawTo(bx+0*scale,by+0*scale);    
    lift(0);
    drawTo(bx+0*scale,by+20*scale);
    drawTo(bx+12*scale,by+0*scale);
    drawTo(bx+12*scale,by+20*scale);
    lift(1);
    break;

  case 'O':
    drawTo(bx+0*scale,by+5*scale);
    lift(0);
    drawTo(bx+0*scale,by+15*scale);
    drawTo(bx+3*scale,by+20*scale);
    drawTo(bx+9*scale,by+20*scale);
    drawTo(bx+12*scale,by+15*scale);
    drawTo(bx+12*scale,by+5*scale);
    drawTo(bx+9*scale,by+0*scale);
    drawTo(bx+3*scale,by+0*scale);
    drawTo(bx+0*scale,by+5*scale);
    lift(1);
    break;

  case 'P':
    drawTo(bx+0*scale,by+0*scale);    
    lift(0);
    drawTo(bx+0*scale,by+20*scale);
    drawTo(bx+9*scale,by+20*scale);
    drawTo(bx+12*scale,by+17*scale);
    drawTo(bx+12*scale,by+13*scale);
    drawTo(bx+9*scale,by+10*scale);
    drawTo(bx+0*scale,by+10*scale);
    lift(1);
    break;

  case 'Q':    
    drawTo(bx+0*scale,by+5*scale);
    lift(0);
    drawTo(bx+0*scale,by+15*scale);
    drawTo(bx+3*scale,by+20*scale);
    drawTo(bx+9*scale,by+20*scale);
    drawTo(bx+12*scale,by+15*scale);
    drawTo(bx+12*scale,by+5*scale);
    drawTo(bx+9*scale,by+0*scale);
    drawTo(bx+3*scale,by+0*scale);
    drawTo(bx+0*scale,by+5*scale);
    lift(1);
    drawTo(bx+9*scale,by+5*scale);
    lift(0);
    drawTo(bx+12*scale,by+0*scale);
    lift(1);
    break;

  case 'R':
    drawTo(bx+0*scale,by+0*scale);       
    lift(0);
    drawTo(bx+0*scale,by+20*scale);
    drawTo(bx+9*scale,by+20*scale);
    drawTo(bx+12*scale,by+17*scale);
    drawTo(bx+12*scale,by+13*scale);
    drawTo(bx+9*scale,by+10*scale);
    drawTo(bx+0*scale,by+10*scale);
    drawTo(bx+12*scale,by+0*scale);
    lift(1);
    break;

  case 'S':
    drawTo(bx+0*scale,by+0*scale);         
    lift(0);
    drawTo(bx+9*scale,by+0*scale);
    drawTo(bx+12*scale,by+3*scale);
    drawTo(bx+12*scale,by+7*scale);
    drawTo(bx+9*scale,by+10*scale);
    drawTo(bx+3*scale,by+10*scale);
    drawTo(bx+0*scale,by+13*scale);
    drawTo(bx+0*scale,by+17*scale);
    drawTo(bx+3*scale,by+20*scale);
    drawTo(bx+12*scale,by+20*scale);
    lift(1);
    break;

  case 'T':   
    drawTo(bx+0*scale,by+20*scale);
    lift(0);
    drawTo(bx+12*scale,by+20*scale);
    lift(1);
    drawTo(bx+6*scale,by+20*scale);
    lift(0);
    drawTo(bx+6*scale,by+0*scale);
    lift(1);
    break;

  case 'U':    
    drawTo(bx+0*scale,by+20*scale);
    lift(0);
    drawTo(bx+0*scale,by+5*scale);
    drawTo(bx+3*scale,by+0*scale);
    drawTo(bx+9*scale,by+0*scale);
    drawTo(bx+12*scale,by+5*scale);
    drawTo(bx+12*scale,by+20*scale);
    lift(1);
    break;

  case 'V':     
    drawTo(bx+0*scale,by+20*scale);
    lift(0);
    drawTo(bx+6*scale,by+0*scale);
    drawTo(bx+12*scale,by+20*scale);
    lift(1);
    break;

  case 'W':     
    drawTo(bx+0*scale,by+20*scale);
    lift(0);
    drawTo(bx+3*scale,by+0*scale);
    drawTo(bx+6*scale,by+10*scale);
    drawTo(bx+9*scale,by+0*scale);
    drawTo(bx+12*scale,by+20*scale);
    lift(1);
    break;

  case 'X':
    drawTo(bx+0*scale,by+20*scale);
    lift(0);
    drawTo(bx+3*scale,by+0*scale);
    drawTo(bx+6*scale,by+10*scale);
    drawTo(bx+9*scale,by+0*scale);
    drawTo(bx+12*scale,by+20*scale);
    lift(1);
    break;

  case 'Y':
    drawTo(bx+0*scale,by+0*scale);      
    lift(0);
    drawTo(bx+12*scale,by+20*scale);
    lift(1);
    drawTo(bx+0*scale,by+20*scale);
    lift(0);
    drawTo(bx+12*scale,by+0*scale);
    lift(1);
    break;   

  case 'Z':
    drawTo(bx+0*scale,by+20*scale);  
    lift(0);
    drawTo(bx+12*scale,by+20*scale);
    drawTo(bx+0*scale,by+0*scale);
    drawTo(bx+12*scale,by+0*scale);
    lift(1);
    break;

  case 'SOL':   
    drawTo(bx+2.5*scale,by+11*scale);  
    lift(0);
    drawTo(bx+2.5*scale,by+9*scale);
    drawTo(bx+3*scale,by+8*scale);
    drawTo(bx+5*scale,by+7*scale);
    drawTo(bx+9*scale,by+8*scale);
    drawTo(bx+9.5*scale,by+9*scale);
    drawTo(bx+9.5*scale,by+11*scale);
    drawTo(bx+9*scale,by+12*scale);
    drawTo(bx+7*scale,by+13*scale);
    drawTo(bx+5*scale,by+13*scale);
    drawTo(bx+3*scale,by+12*scale);
    drawTo(bx+2.5*scale,by+11*scale);
    drawTo(bx+2.5*scale,by+9*scale);
    lift(1);
    drawTo(bx+0*scale,by+10*scale);
    lift(0);
    drawTo(bx+2.5*scale,by+10*scale);
    lift(1);
    drawTo(bx+1*scale,by+13*scale);
    lift(0);
    drawTo(bx+3*scale,by+12*scale);
    lift(1);
    drawTo(bx+4*scale,by+15*scale);
    lift(0);
    drawTo(bx+5*scale,by+13*scale);
    lift(1);
    drawTo(bx+8*scale,by+15*scale);
    lift(0);
    drawTo(bx+7*scale,by+13*scale);
    lift(1);
    drawTo(bx+11*scale,by+13*scale);
    lift(0);
    drawTo(bx+9*scale,by+12*scale);
    lift(1);
    drawTo(bx+12*scale,by+10*scale);
    lift(0);
    drawTo(bx+9.5*scale,by+10*scale);
    lift(1);
    drawTo(bx+11*scale,by+7*scale);
    lift(0);
    drawTo(bx+9*scale,by+8*scale);
    lift(1);
    drawTo(bx+8*scale,by+5*scale);
    lift(0);
    drawTo(bx+7*scale,by+7*scale);
    lift(1);
    drawTo(bx+4*scale,by+5*scale);
    lift(0);
    drawTo(bx+5*scale,by+7*scale);
    lift(1);
    drawTo(bx+1*scale,by+7*scale);
    lift(0);
    drawTo(bx+3*scale,by+8*scale);
    lift(1);
    break;

  case 'sky':
    drawTo(bx+0*scale,by+10*scale);  
    lift(0);
    drawTo(bx+0*scale,by+11*scale);
    drawTo(bx+2*scale,by+12.5*scale);
    drawTo(bx+3*scale,by+12*scale);
    drawTo(bx+3.5*scale,by+13*scale);
    drawTo(bx+4.5*scale,by+14*scale);
    drawTo(bx+6*scale,by+14*scale);
    drawTo(bx+7*scale,by+13*scale);
    drawTo(bx+8*scale,by+14*scale);
    drawTo(bx+9*scale,by+14*scale);
    drawTo(bx+11*scale,by+13*scale);
    drawTo(bx+12*scale,by+11*scale);
    drawTo(bx+12*scale,by+10*scale);
    drawTo(bx+11*scale,by+9*scale);
    drawTo(bx+1*scale,by+9*scale);
    drawTo(bx+0*scale,by+10*scale);
    lift(1);
    break;

  case 'regnsky':
    drawTo(bx+0*scale,by+10*scale);  
    lift(0);
    drawTo(bx+0*scale,by+11*scale);
    drawTo(bx+2*scale,by+12.5*scale);
    drawTo(bx+3*scale,by+12*scale);
    drawTo(bx+3.5*scale,by+13*scale);
    drawTo(bx+4.5*scale,by+14*scale);
    drawTo(bx+6*scale,by+14*scale);
    drawTo(bx+7*scale,by+13*scale);
    drawTo(bx+8*scale,by+14*scale);
    drawTo(bx+9*scale,by+14*scale);
    drawTo(bx+11*scale,by+13*scale);
    drawTo(bx+12*scale,by+11*scale);
    drawTo(bx+12*scale,by+10*scale);
    drawTo(bx+11*scale,by+9*scale);
    drawTo(bx+1*scale,by+9*scale);
    drawTo(bx+0*scale,by+10*scale);
    lift(1);
    drawTo(bx+2*scale,by+8*scale);  
    lift(0);
    drawTo(bx+2*scale,by+7*scale);
    lift(1);
    drawTo(bx+3*scale,by+7*scale);  
    lift(0);
    drawTo(bx+3*scale,by+6*scale);
    lift(1);
    drawTo(bx+4*scale,by+8*scale);  
    lift(0);
    drawTo(bx+4*scale,by+7*scale);
    lift(1);
    drawTo(bx+5*scale,by+7*scale);  
    lift(0);
    drawTo(bx+5*scale,by+6*scale);
    lift(1);
    drawTo(bx+6*scale,by+8*scale);  
    lift(0);
    drawTo(bx+6*scale,by+7*scale);
    lift(1);
    drawTo(bx+7*scale,by+7*scale);  
    lift(0);
    drawTo(bx+7*scale,by+6*scale);
    lift(1);
    drawTo(bx+8*scale,by+8*scale);  
    lift(0);
    drawTo(bx+8*scale,by+7*scale);
    lift(1);
    drawTo(bx+9*scale,by+7*scale);  
    lift(0);
    drawTo(bx+9*scale,by+6*scale);
    lift(1);
    drawTo(bx+10*scale,by+8*scale);  
    lift(0);
    drawTo(bx+10*scale,by+7*scale);
    lift(1);
    break;
    
  }
}

void lift(char lift) {
  switch (lift) {
    // room to optimize  !

  case 0: //850

      if (servoLift >= LIFT0) {
      while (servoLift >= LIFT0) 
      {
        servoLift--;
        servo1.writeMicroseconds(servoLift);        
        delayMicroseconds(LIFTSPEED);
      }
    } 
    else {
      while (servoLift <= LIFT0) {
        servoLift++;
        servo1.writeMicroseconds(servoLift);
        delayMicroseconds(LIFTSPEED);

      }

    }

    break;

  case 1: //150

    if (servoLift >= LIFT1) {
      while (servoLift >= LIFT1) {
        servoLift--;
        servo1.writeMicroseconds(servoLift);
        delayMicroseconds(LIFTSPEED);

      }
    } 
    else {
      while (servoLift <= LIFT1) {
        servoLift++;
        servo1.writeMicroseconds(servoLift);
        delayMicroseconds(LIFTSPEED);
      }

    }

    break;

  case 2:

    if (servoLift >= LIFT2) {
      while (servoLift >= LIFT2) {
        servoLift--;
        servo1.writeMicroseconds(servoLift);
        delayMicroseconds(LIFTSPEED);
      }
    } 
    else {
      while (servoLift <= LIFT2) {
        servoLift++;
        servo1.writeMicroseconds(servoLift);        
        delayMicroseconds(LIFTSPEED);
      }
    }
    break;
  }
}


void bogenUZS(float bx, float by, float radius, int start, int ende, float sqee) {
  float inkr = -0.05;
  float count = 0;

  do {
    drawTo(sqee * radius * cos(start + count) + bx,
    radius * sin(start + count) + by);
    count += inkr;
  } 
  while ((start + count) > ende);

}

void bogenGZS(float bx, float by, float radius, int start, int ende, float sqee) {
  float inkr = 0.05;
  float count = 0;

  do {
    drawTo(sqee * radius * cos(start + count) + bx,
    radius * sin(start + count) + by);
    count += inkr;
  } 
  while ((start + count) <= ende);
}


void drawTo(double pX, double pY) {
  double dx, dy, c;
  int i;

  // dx dy of new point
  dx = pX - lastX;
  dy = pY - lastY;
  //path lenght in mm, times 4 equals 4 steps per mm
  c = floor(7 * sqrt(dx * dx + dy * dy));

  if (c < 1) c = 1;

  for (i = 0; i <= c; i++) {
    // draw line point by point
    set_XY(lastX + (i * dx / c), lastY + (i * dy / c));

  }

  lastX = pX;
  lastY = pY;
}

double return_angle(double a, double b, double c) {
  // cosine rule for angle between c and a
  return acos((a * a + c * c - b * b) / (2 * a * c));
}

void set_XY(double Tx, double Ty) 
{
  delay(1);
  double dx, dy, c, a1, a2, Hx, Hy;

  // calculate triangle between pen, servoLeft and arm joint
  // cartesian dx/dy
  dx = Tx - O1X;
  dy = Ty - O1Y;

  // polar lemgth (c) and angle (a1)
  c = sqrt(dx * dx + dy * dy); // 
  a1 = atan2(dy, dx); //
  a2 = return_angle(L1, L2, c);

  servo2.writeMicroseconds(floor(((a2 + a1 - M_PI) * SERVOFAKTOR) + SERVOLEFTNULL));

  // calculate joinr arm point for triangle of the right servo arm
  a2 = return_angle(L2, L1, c);
  Hx = Tx + L3 * cos((a1 - a2 + 0.621) + M_PI); //36,5°
  Hy = Ty + L3 * sin((a1 - a2 + 0.621) + M_PI);

  // calculate triangle between pen joint, servoRight and arm joint
  dx = Hx - O2X;
  dy = Hy - O2Y;

  c = sqrt(dx * dx + dy * dy);
  a1 = atan2(dy, dx);
  a2 = return_angle(L1, L2, c);

  servo3.writeMicroseconds(floor(((a1 - a2) * SERVOFAKTOR) + SERVORIGHTNULL));

}


//-------------------------------------------------------------node-red funktioner----------------------------------------------


void callback(char* topic, byte* payload, unsigned int payloadLength) { //Handle json callback from a topic we subscribe to
  Serial.print("callback invoked for topic: "); Serial.println(topic);

  StaticJsonBuffer<300> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject((char*)payload, payloadLength);
  if (!root.success()) {
    Serial.print("callback: payload parse FAILED: ");
    Serial.print(payloadLength); Serial.print(":"); Serial.print((char*)payload);
    return;
  }

  else if (root.success()) {
  Serial.print("callback: payload parse OK: "); root.printTo(Serial); Serial.println();
  }
  if (strcmp (commandTopic, topic) == 0) {
    handleCommand(root);
    Serial.println("comandT - her");
    
  } else {
    Serial.print("unexpected callback: ");
    Serial.println(topic);
  }
}

///// Print til serial monitor /////

void publishTo(const char* topic, const char* payload) { //Publish to a specific MQTT topic
  Serial.print("publish ");
  if (client.publish(topic, payload)) {
    Serial.print(" OK ");
  } else {
    Serial.print(" FAILED ");
  }
  Serial.print(" topic: "); Serial.print(topic);
  Serial.print(" payload: "); Serial.println(payload);
}

void subscribeTo(const char* topic) { //Subscribe to a specific MQTT topic on the Broker
  Serial.print("subscribe ");
  if (client.subscribe(topic)) {
    Serial.print(" OK ");
  } else {
    Serial.print(" FAILED ");
  }
  Serial.print(" to: "); Serial.println(topic);
}



void wifiConnect() { //Connect to wifi
  Serial.print("Connecting to "); Serial.print(ssid);
  WiFi.begin(ssid, password);
  //WiFi.begin(“device");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("\nWiFi connected, IP address: "); Serial.println(WiFi.localIP());
}

void mqttConnect() { //Connect to MQTT Broker
  if (!!!client.connected()) {
    Serial.print("Reconnecting MQTT client to "); Serial.println(server);
    while (!!!client.connect(clientId, authMethod, token)) {
      Serial.print(".");
      delay(500);
    }
    Serial.println();
  }
}

//-----------------------------------------------------------Stepper ------------------------------------------
void drej() {
  M = nextM;
  Omk = startOmk + nextOmk;

  turns(x / Omk);


  // Omkredsen bliver længere, og antal omdrejninger bliver større.
  nextM = M + (x / Omk);
  bonusOmk = pi * t * M;
  nextOmk =  bonusOmk;
}

void turns(float rotations) {
  // if the rotation count is -ve then it is CCW
  Serial.println();
  Serial.print("Turning : ");
  Serial.print(rotations);
  Serial.println(" rotations");
  bool clockwise = rotations > 0;
  Serial.print("Clockwise = ");
  Serial.println(clockwise);
  // calculate how many cycles the stepper will have to make
  int cycles = rotations * CYCLES_PER_ROTATION;
  // force the cycle count to be positive
  cycles = abs(cycles);
  Serial.print("That is ");
  Serial.print(cycles);
  Serial.print(" Cycles ");
  // only move if the user specifed an actual movement
  if (rotations != 0)
  {
    if (clockwise)
    {
      Serial.println("Clockwise");
      // for each cycle
      for (int x = 0; x < cycles; x++)
      {
        // for each phase
        for (int y = 0; y < 8; y++)
        {
          // go to phase y
          phaseSelect(y);
          // pause so the stepper has time to react
          delay(1);
        }
      }
    } else {
      Serial.println("Counter Clockwise");
      // for each cycle
      for (int x = 0; x < cycles; x++)
      {
        // for each phase (backwards for CCW rotation)
        for (int y = 7; y >= 0; y--)
        {
          // go to phase y
          phaseSelect(y);
          // pause so the stepper has time to react
          delay(1);
        }
      }
    }
  }
  // go to the default state (all poles off) when finished
  phaseSelect(8);
  Serial.println("Done");
}

void phaseSelect(int phase)
{
  switch (phase) {
    case 0:
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, HIGH);
      break;
    case 1:
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, HIGH);
      digitalWrite(IN4, HIGH);
      break;
    case 2:
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, HIGH);
      digitalWrite(IN4, LOW);
      break;
    case 3:
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, HIGH);
      digitalWrite(IN3, HIGH);
      digitalWrite(IN4, LOW);
      break;
    case 4:
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, HIGH);
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, LOW);
      break;
    case 5:
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, HIGH);
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, LOW);
      break;
    case 6:
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, LOW);
      break;
    case 7:
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, HIGH);
      break;
    default:
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, LOW);
      break;
  }
}


