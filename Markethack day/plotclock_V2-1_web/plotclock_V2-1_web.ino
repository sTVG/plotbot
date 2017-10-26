// Plotclock
// cc - by Johannes Heberlein 2014
// v 1.01
// thingiverse.com/joo   wiki.fablab-nuernberg.de

// units: mm; microseconds; radians
// origin: bottom left of drawing surface

// time library see http://playground.arduino.cc/Code/time 

// delete or mark the next line as comment when done with calibration  
//#define CALIBRATION

// When in calibration mode, adjust the following factor until the servos move exactly 90 degrees
//#define SERVOFAKTOR 670

// Zero-position of left and right servo
// When in calibration mode, adjust the NULL-values so that the servo arms are at all times parallel
// either to the X or Y axis
//#define SERVOLEFTNULL 2250
//#define SERVORIGHTNULL 1200

//#define SERVOPINLIFT  16
//#define SERVOPINLEFT  14
//#define SERVOPINRIGHT 5

// lift positions of lifting servo
//#define LIFT0 1550 // on drawing surface
//#define LIFT1 1250  // between numbers
#define LIFT2 1100  // going towards sweeper

// speed of liftimg arm, higher is slower
#define LIFTSPEED 1500

// length of arms
#define L1 35
#define L2 55
#define L3 13


// origin points of left and right servo 
#define O1X 22
#define O1Y -25
#define O2X 47
#define O2Y -25


#define EEPROM_STATUS_ADDRESS 0
#define EEPROM_SERVOFAKTOR_ADDRESS 2
#define EEPROM_LEFTNULL_ADDRESS 4
#define EEPROM_RIGHTNULL_ADDRESS 6
#define EEPROM_LIFT0_ADDRESS 8
#define EEPROM_LIFT1_ADDRESS 10


int SERVOFAKTOR = 670;
int SERVOLEFTNULL  = 2200;
int SERVORIGHTNULL = 1050;

int SERVOPINLIFT = 16;
int SERVOPINLEFT = 14;
int SERVOPINRIGHT = 5;

int LIFT0 = 1550; // on drawing surface
int LIFT1 = 1300;  // between numbers




// wifi manager stuff
#include <ESP8266WiFi.h>          //ESP8266 Core WiFi Library (you most likely already have this in your sketch)

//#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
//#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
//#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic


#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
 //ESP8266WebServer server(80);

#include <Time.h> // see http://playground.arduino.cc/Code/time 
#include <Servo.h>
#include <TimeLib.h>

int servoLift = 1500;
// MODES - both off = time mode 
bool ModeCalibrate = false;
bool ModeText = false;


Servo servo1;  // 
Servo servo2;  // 
Servo servo3;  // 

volatile double lastX = 75;
volatile double lastY = 47.5;

int last_min = 0;

  MDNSResponder mdns;
  // Replace with your network credentials
  const char* ssid = "Grays Home";
  const char* password = "community";

 // ESP8266WebServer server(80);

  String webPage = "";

  int gpio0_pin = 0;
  int gpio2_pin = 2;

  ESP8266WebServer server(80);

void setup() 
{ 

  // do we have a saved EEPROM? 
  int ans = readEEPROM(EEPROM_STATUS_ADDRESS);
  if(ans != 123) {
    // No EEPROM - So Write default Values 
    writeDefaultEEPROM();
  } else {
    // load values from eeprom 
    SERVOFAKTOR = readEEPROM(EEPROM_SERVOFAKTOR_ADDRESS);
    SERVOLEFTNULL  = readEEPROM(EEPROM_LEFTNULL_ADDRESS);
    SERVORIGHTNULL = readEEPROM(EEPROM_RIGHTNULL_ADDRESS);
    LIFT0 = readEEPROM(EEPROM_LIFT0_ADDRESS);
    LIFT1 = readEEPROM(EEPROM_LIFT1_ADDRESS);
    
  }


  webPage = "<h1>ESP8266 Web Server  PlotBot </h1>";
  webPage += "<p>CALIBRATION MODE  [" + boolToString(ModeCalibrate) + "]  <a href=\"CalibrateOn\"><button>On</button></a>&nbsp;<a href=\"CalibrateOff\"><button>Off</button></a></p>";
  webPage += "<p>TEXT MODE  [" + boolToString(ModeText) + "]  <a href=\"TextOn\"><button>On</button></a>&nbsp;<a href=\"TextOff\"><button>Off</button></a></p>";
  webPage += "<p>SERVO FAKTOR [" + toString(SERVOFAKTOR) + "]  <a href=\"faktorPlus\"><button>Plus</button></a>&nbsp;<a href=\"faktorMinus\"><button>Minus</button></a></p>";
  webPage += "<p>LEFT SERVO NULL [" + toString(SERVOLEFTNULL) + "]  <a href=\"leftServoNullPlus\"><button>Plus</button></a>&nbsp;<a href=\"leftServoNullMinus\"><button>Minus</button></a></p>";
  webPage += "<p>RIGHT SERVO NULL [" + toString(SERVORIGHTNULL) + "]  <a href=\"rightServoNullPlus\"><button>Plus</button></a>&nbsp;<a href=\"rightServoNullMinus\"><button>Minus</button></a></p>";
  webPage += "<p>LIFT SERVO - DOWN [" + toString(LIFT0) + "]  <a href=\"lift0Plus\"><button>Plus</button></a>&nbsp;<a href=\"lift0Minus\"><button>Minus</button></a></p>";
  webPage += "<p>LIFT SERVO - UP [" + toString(LIFT1) + "]  <a href=\"lift1Plus\"><button>Plus</button></a>&nbsp;<a href=\"lift1Minus\"><button>Minus</button></a></p>";


  
 
  //String webPage = "";

 
  // Set current time only the first to values, hh,mm are needed
  setTime(19,38,0,0,0,0);

  drawTo(75.2, 47);
  lift(0);
  servo1.attach(SERVOPINLIFT);  //  lifting servo
  servo2.attach(SERVOPINLEFT);  //  left servo
  servo3.attach(SERVOPINRIGHT);  //  right servo
  delay(1000);

// WI FI SERVER STARING SETUP 
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  if (mdns.begin("esp8266", WiFi.localIP())) {
    Serial.println("MDNS responder started");
  }
  
  server.on("/", [](){
    server.send(200, "text/html", webPage);
  });
  // MODE CALIBRATE ------------------------------------
  server.on("/CalibrateOn", [](){
    ModeCalibrate = true;
    ModeText = false;
    server.send(200, "text/html", webPage);
    delay(1000);
  });
  server.on("/CalibrateOff", [](){
    ModeCalibrate = false;
    // write EEPROM values
    writeDefaultEEPROM();
    server.send(200, "text/html", webPage);
    delay(1000);
  });
  // MODE TEXT ------------------------------------
  server.on("/TextOn", [](){
    ModeText = true;
    ModeCalibrate = false;
    server.send(200, "text/html", webPage);
    delay(1000);
  });
  server.on("/TextOff", [](){
    ModeCalibrate = false;
    server.send(200, "text/html", webPage);
    delay(1000);
  });
  // FAKTOR ---------------------------------------------
  server.on("/faktorPlus", [](){
    SERVOFAKTOR  = SERVOFAKTOR + 50;
    server.send(200, "text/html", webPage);
    delay(1000); 
  });
  server.on("/faktorMinus", [](){
    SERVOFAKTOR  = SERVOFAKTOR - 50;
    server.send(200, "text/html", webPage);
    delay(1000); 
  });
  // LEFT SERVO NULL ---------------------------------------------------
  server.on("/leftServoNullPlus", [](){
    SERVOLEFTNULL = SERVOLEFTNULL + 50;
    server.send(200, "text/html", webPage);
    delay(1000);
  });
  server.on("/leftServoNullMinus", [](){
    SERVOLEFTNULL = SERVOLEFTNULL - 50;
    server.send(200, "text/html", webPage);
    delay(1000);
  });
  // RIGHTT SERVO NULL ---------------------------------------------------
  server.on("/rightServoNullPlus", [](){
    SERVORIGHTNULL = SERVORIGHTNULL + 50;
    server.send(200, "text/html", webPage);
    delay(1000);
  });
  server.on("/rightServoNullMinus", [](){
    SERVORIGHTNULL = SERVORIGHTNULL - 50;
    server.send(200, "text/html", webPage);
    delay(1000);
  });  
  // Lift 0 ---------------------------------------------------------------
  server.on("/lift0Plus", [](){
    LIFT0 = LIFT0 + 25;
    server.send(200, "text/html", webPage);
    delay(1000); 
  });
  server.on("/lift0Minus", [](){
    LIFT0 = LIFT0 - 25;
    server.send(200, "text/html", webPage);
    delay(1000); 
  });
  // Lift 1 ---------------------------------------------------------------
  server.on("/lift1Plus", [](){
    LIFT1 = LIFT1 + 25;
    server.send(200, "text/html", webPage);
    delay(1000); 
  });
  server.on("/lift1Minus", [](){
    LIFT1 = LIFT1 - 25;
    server.send(200, "text/html", webPage);
    delay(1000); 
  });

  server.begin();
  Serial.println("HTTP server started");

// delay(1000);
pinMode(15, OUTPUT);
pinMode(12, OUTPUT);
pinMode(13, OUTPUT);

delay(1000);
  digitalWrite(15, LOW);
  digitalWrite(12, LOW);
  digitalWrite(13, HIGH);

  delay(2000);
   digitalWrite(15, LOW);
  digitalWrite(12, LOW);
  digitalWrite(13, LOW);


} 

void loop() 
{ 

  webPage = "<h1>ESP8266 Web Server  PlotBot </h1>";
  webPage += "<p>CALIBRATION MODE  [" + boolToString(ModeCalibrate) + "]  <a href=\"CalibrateOn\"><button>On</button></a>&nbsp;<a href=\"CalibrateOff\"><button>Off</button></a></p>";
  webPage += "<p>TEXT MODE  [" + boolToString(ModeText) + "]  <a href=\"TextOn\"><button>On</button></a>&nbsp;<a href=\"TextOff\"><button>Off</button></a></p>";
  webPage += "<p>SERVO FAKTOR [" + toString(SERVOFAKTOR) + "]  <a href=\"faktorPlus\"><button>Plus</button></a>&nbsp;<a href=\"faktorMinus\"><button>Minus</button></a></p>";
  webPage += "<p>LEFT SERVO NULL [" + toString(SERVOLEFTNULL) + "]  <a href=\"leftServoNullPlus\"><button>Plus</button></a>&nbsp;<a href=\"leftServoNullMinus\"><button>Minus</button></a></p>";
  webPage += "<p>RIGHT SERVO NULL [" + toString(SERVORIGHTNULL) + "]  <a href=\"rightServoNullPlus\"><button>Plus</button></a>&nbsp;<a href=\"rightServoNullMinus\"><button>Minus</button></a></p>";
  webPage += "<p>LIFT SERVO - DOWN [" + toString(LIFT0) + "]  <a href=\"lift0Plus\"><button>Plus</button></a>&nbsp;<a href=\"lift0Minus\"><button>Minus</button></a></p>";
  webPage += "<p>LIFT SERVO - UP [" + toString(LIFT1) + "]  <a href=\"lift1Plus\"><button>Plus</button></a>&nbsp;<a href=\"lift1Minus\"><button>Minus</button></a></p>";

  server.handleClient();
  
if(ModeCalibrate){
//#ifdef CALIBRATION

    if (!servo1.attached()) servo1.attach(SERVOPINLIFT);
    if (!servo2.attached()) servo2.attach(SERVOPINLEFT);
    if (!servo3.attached()) servo3.attach(SERVOPINRIGHT);


  
  Serial.println("Calibrate  mode  - 8 high,  7 low, 6 low");
  digitalWrite(15, HIGH);
  digitalWrite(12, LOW);
  digitalWrite(13, LOW);
  delay(200);
  digitalWrite(15, LOW);
  digitalWrite(12, LOW);
  digitalWrite(13, LOW);
  delay(200);

  //lift(0);
  //number(5, 25, 8, 0.9);
  //delay(1000);
  // Servohorns will have 90° between movements, parallel to x and y axis
  drawTo(-3, 29.2);
  delay(500);
  drawTo(74.1, 28);
  delay(500);
  lift(0);
  delay(1000);
  lift(1);
  
} else if (ModeText) {
      // TEXT MODE
  digitalWrite(15, LOW);
  digitalWrite(12, LOW);
  digitalWrite(13, HIGH);

  lift(0);
    drawTo(0,0);
    
    lift(0);// Get ready to write   
    number(5, 10, 'h', 0.);  // first character

    number(15, 10, 'e', 0.4);  // first character

    number(25, 10, 'l', 0.4);  // first character

    number(35, 10, 'l', 0.4);  // first character
 
    number(45, 10, '0', 0.4);  // first character
   

 
    lift(1);

    delay(2000);


} else {

    // this is Time Mode !!! 
  
//#else

  //Serial.println("Not Calibrate mode - 8 low, 7 high , 6 low");
  digitalWrite(15, LOW);
  digitalWrite(12, HIGH);
  digitalWrite(13, LOW);


  int i = 0;
  if (last_min != minute()) {

    if (!servo1.attached()) servo1.attach(SERVOPINLIFT);
    if (!servo2.attached()) servo2.attach(SERVOPINLEFT);
    if (!servo3.attached()) servo3.attach(SERVOPINRIGHT);

    lift(0);

    hour();
    while ((i+1)*10 <= hour())
    {
      i++;
    }

   // number(3, 3, 111, 1);
    number(5, 25, i, 0.9);
    number(19, 25, (hour()-i*10), 0.9);
    number(28, 25, 11, 0.9);

    i=0;
    while ((i+1)*10 <= minute())
    {
      i++;
    }
    number(34, 25, i, 0.9);
    number(48, 25, (minute()-i*10), 0.9);
    lift(2);
    drawTo(74.2, 47.5);
    lift(1);
    last_min = minute();

    servo1.detach();
    servo2.detach();
    servo3.detach();
  }
}
//#endif



} 

// Writing numeral with bx by being the bottom left originpoint. Scale 1 equals a 20 mm high font.
// The structure follows this principle: move to first startpoint of the numeral, lift down, draw numeral, lift up
void number(float bx, float by, int num, float scale) {

  switch (num) {

  case 0:
    drawTo(bx + 12 * scale, by + 6 * scale);
    lift(0);
    bogenGZS(bx + 7 * scale, by + 10 * scale, 10 * scale, -0.8, 6.7, 0.5);
    lift(1);
    break;
  case 1:

    drawTo(bx + 3 * scale, by + 15 * scale);
    lift(0);
    drawTo(bx + 10 * scale, by + 20 * scale);
    drawTo(bx + 10 * scale, by + 0 * scale);
    lift(1);
    break;
  case 2:
    drawTo(bx + 2 * scale, by + 12 * scale);
    lift(0);
    bogenUZS(bx + 8 * scale, by + 14 * scale, 6 * scale, 3, -0.8, 1);
    drawTo(bx + 1 * scale, by + 0 * scale);
    drawTo(bx + 12 * scale, by + 0 * scale);
    lift(1);
    break;
  case 3:
    drawTo(bx + 2 * scale, by + 17 * scale);
    lift(0);
    bogenUZS(bx + 5 * scale, by + 15 * scale, 5 * scale, 3, -2, 1);
    bogenUZS(bx + 5 * scale, by + 5 * scale, 5 * scale, 1.57, -3, 1);
    lift(1);
    break;
  case 4:
    drawTo(bx + 10 * scale, by + 0 * scale);
    lift(0);
    drawTo(bx + 10 * scale, by + 20 * scale);
    drawTo(bx + 2 * scale, by + 6 * scale);
    drawTo(bx + 12 * scale, by + 6 * scale);
    lift(1);
    break;
  case 5:
    drawTo(bx + 2 * scale, by + 5 * scale);
    lift(0);
    bogenGZS(bx + 5 * scale, by + 6 * scale, 6 * scale, -2.5, 2, 1);
    drawTo(bx + 5 * scale, by + 20 * scale);
    drawTo(bx + 12 * scale, by + 20 * scale);
    lift(1);
    break;
  case 6:
    drawTo(bx + 2 * scale, by + 10 * scale);
    lift(0);
    bogenUZS(bx + 7 * scale, by + 6 * scale, 6 * scale, 2, -4.4, 1);
    drawTo(bx + 11 * scale, by + 20 * scale);
    lift(1);
    break;
  case 7:
    drawTo(bx + 2 * scale, by + 20 * scale);
    lift(0);
    drawTo(bx + 12 * scale, by + 20 * scale);
    drawTo(bx + 2 * scale, by + 0);
    lift(1);
    break;
  case 8:
    drawTo(bx + 5 * scale, by + 10 * scale);
    lift(0);
    bogenUZS(bx + 5 * scale, by + 15 * scale, 5 * scale, 4.7, -1.6, 1);
    bogenGZS(bx + 5 * scale, by + 5 * scale, 5 * scale, -4.7, 2, 1);
    lift(1);
    break;

  case 9:
    drawTo(bx + 9 * scale, by + 11 * scale);
    lift(0);
    bogenUZS(bx + 7 * scale, by + 15 * scale, 5 * scale, 4, -0.5, 1);
    drawTo(bx + 5 * scale, by + 0);
    lift(1);
    break;

  case 111:

    lift(0);
    drawTo(70, 46);
    drawTo(65, 43);

    drawTo(65, 49);
    drawTo(5, 49);
    drawTo(5, 45);
    drawTo(65, 45);
    drawTo(65, 40);

    drawTo(5, 40);
    drawTo(5, 35);
    drawTo(65, 35);
    drawTo(65, 30);

    drawTo(5, 30);
    drawTo(5, 25);
    drawTo(65, 25);
    drawTo(65, 20);

    drawTo(5, 20);
    drawTo(60, 44);

    drawTo(75.2, 47);
    lift(2);

    break;

  case 11:
    drawTo(bx + 5 * scale, by + 15 * scale);
    lift(0);
    bogenGZS(bx + 5 * scale, by + 15 * scale, 0.1 * scale, 1, -1, 1);
    lift(1);
    drawTo(bx + 5 * scale, by + 5 * scale);
    lift(0);
    bogenGZS(bx + 5 * scale, by + 5 * scale, 0.1 * scale, 1, -1, 1);
    lift(1);
    break;
    
case '?':
      drawTo(bx + 5 * scale, by + 15 * scale);
      lift(0);
      bogenGZS(bx + 5 * scale, by + 15 * scale, 0.1 * scale, 1, -1, 1);
      lift(1);
      drawTo(bx + 5 * scale, by + 5 * scale);
      lift(0);
      bogenGZS(bx + 5 * scale, by + 5 * scale, 0.1 * scale, 1, -1, 1);
      lift(1);
      break;
    case 'a':
    case 'A':
      drawTo(bx, by);                           // 0,0
      lift(0);
      drawTo(bx , by + 20 * scale);             // 0,20
      drawTo(bx + 10 * scale, by + 20 * scale); // 10,20
      drawTo(bx + 10 * scale, by);              // 10,0
      lift(1);
      drawTo(bx + 10 * scale, by + 10 * scale); // 10,10
      lift(0);
      drawTo(bx, by + 10 * scale);              // 0,10
      lift(1);
      break;
    case 'b':
    case 'B':
      drawTo(bx, by + 10 * scale);              // 0,10
      lift(0);
      drawTo(bx + 10 * scale, by + 10 * scale); // 10,10
      lift(1);
      goto drawO; // use the same steps to draw the outside.V
      break;
    case 'c':
    case 'C':
      drawTo(bx + 10 * scale, by);              // 10,0
      lift(0);
      drawTo(bx, by);                           // 0,0
      drawTo(bx , by + 20 * scale);             // 0,20
      drawTo(bx + 10 * scale, by + 20 * scale); // 10,20
      lift(1);
      break;
    case 'd':
    case 'D':
      drawTo(bx + 10 * scale, by + 20 * scale); // 10,20
      lift(0);
      drawTo(bx + 10 * scale, by);              // 10,0
      drawTo(bx, by);                           // 0,0
      drawTo(bx, by + 10 * scale);              // 0,10
      drawTo(bx + 10 * scale, by + 10 * scale); // 10,10
      lift(1);
      break;
    case 'e':
    case 'E':
      drawTo(bx + 10 * scale, by);              // 10,0
      lift(0);
      drawTo(bx, by);                           // 0,0
      drawTo(bx , by + 20 * scale);             // 0,20
      drawTo(bx + 10 * scale, by + 20 * scale); // 10,20
      lift(1);
      drawTo(bx + 10 * scale, by + 10 * scale); // 10,10
      lift(0);
      drawTo(bx, by + 10 * scale);              // 0,10
      lift(1);
      break;
    case 'f':
    case 'F':
      drawTo(bx, by);                           // 0,0
      lift(0);
      drawTo(bx , by + 20 * scale);             // 0,20
      drawTo(bx + 10 * scale, by + 20 * scale); // 10,20
      lift(1);
      drawTo(bx + 10 * scale, by + 10 * scale); // 10,10
      lift(0);
      drawTo(bx, by + 10 * scale);              // 0,10
      lift(1);
      break;
    case 'g':
    case 'G':
      drawTo(bx + 10 * scale, by + 20 * scale); // 10,20
      lift(0);
      drawTo(bx , by + 20 * scale);             // 0,20
      drawTo(bx, by);                           // 0,0
      drawTo(bx + 10 * scale, by);              // 10,0
      drawTo(bx + 10 * scale, by + 10 * scale); // 10,10
      drawTo(bx + 6 * scale, by + 10 * scale);  // 10,10
      lift(1);
      break;
    case 'h':
    case 'H':
      drawTo(bx, by);                           // 0,0
      lift(0);
      drawTo(bx , by + 20 * scale);             // 0,20
      lift(1);
      drawTo(bx + 10 * scale, by + 20 * scale); // 10,20
      lift(0);
      drawTo(bx + 10 * scale, by);              // 10,0
      lift(1);
      drawTo(bx + 10 * scale, by + 10 * scale); // 10,10
      lift(0);
      drawTo(bx, by + 10 * scale);              // 0,10
      lift(1);
      break;
    case 'i':
    case 'I':
      drawTo(bx, by);                           // 0,0
      lift(0);
      drawTo(bx + 10 * scale, by);              // 10,0
      lift(1);
      drawTo(bx + 10 * scale, by + 20 * scale); // 10,20
      lift(0);
      drawTo(bx , by + 20 * scale);             // 0,20
      lift(1);
      drawTo(bx + 5 , by + 20 * scale);         // 5,20
      lift(0);
      drawTo(bx + 5 , by + 0 * scale);          // 5,0
      lift(1);
      break;
    case 'j':
    case 'J':
      drawTo(bx , by + 20 * scale);              // 0,20
      lift(0);
      drawTo(bx + 10 * scale, by + 20 * scale);  // 10,20
      lift(1);
      drawTo(bx + 5 , by + 20 * scale);          // 5,20
      lift(0);
      drawTo(bx + 5 , by + 0 * scale);           // 5,0
      drawTo(bx + 3 , by + 2 * scale);           // 3,2
      lift(1);
      break;
    case 'k':
    case 'K':
      drawTo(bx, by);                           // 0,0
      lift(0);
      drawTo(bx , by + 20 * scale);             // 0,20
      lift(1);
      drawTo(bx + 10 * scale, by + 20 * scale); // 10,20
      lift(0);
      drawTo(bx, by + 10 * scale);              // 0,10
      drawTo(bx + 10 * scale, by);              // 10,0
      lift(1);
      break;
    case 'l':
    case 'L':
      drawTo(bx , by + 20 * scale);             // 0,20
      lift(0);
      drawTo(bx, by);                           // 0,0
      drawTo(bx + 10 * scale, by);              // 10,0
      lift(1);
      break;
    case 'm':
    case 'M':
      drawTo(bx, by);                           // 0,0
      lift(0);
      drawTo(bx , by + 20 * scale);             // 0,20
      drawTo(bx + 5, by + 10 * scale);          // 5,10
      drawTo(bx + 10 * scale, by + 20 * scale); // 10,20
      drawTo(bx + 10 * scale, by);              // 10,0
      lift(1);
      break;
    case 'n':
    case 'N':
      drawTo(bx, by);                           // 0,0
      lift(0);
      drawTo(bx , by + 20 * scale);             // 0,20
      drawTo(bx + 10 * scale, by);              // 10,0
      drawTo(bx + 10 * scale, by + 20 * scale); // 10,20
      lift(1);
      break;
    case 'ox':
    case 'Ox':
drawO:
      lift(1);
      lift(0);
      drawTo(bx , by + 20 * scale);             // 0,20
      drawTo(bx + 10 * scale, by + 20 * scale); // 10,20
      drawTo(bx + 10 * scale, by);              // 10,0
      drawTo(bx, by);                           // 0,0
      lift(1);
      break;
    case 'p':
    case 'P':
drawP:
      drawTo(bx, by);                           // 0,0
      lift(0);
      drawTo(bx , by + 20 * scale);             // 0,20
      drawTo(bx + 10 * scale, by + 20 * scale); // 10,20
      drawTo(bx + 10 * scale, by + 10 * scale); // 10,10
      drawTo(bx, by + 10 * scale);              // 0,10
      lift(1);
      break;
    case 'q':
    case 'Q':
      drawTo(bx + 10 * scale, by + 10 * scale); // 10,10
      lift(0);
      drawTo(bx, by + 10 * scale);              // 0,10
      drawTo(bx , by + 20 * scale);             // 0,20
      drawTo(bx + 10 * scale, by + 20 * scale); // 10,20
      drawTo(bx + 10 * scale, by);              // 10,0
      drawTo(bx + 11 * scale, by + 2);          // 11,2
      break;
    case 'r':
    case 'R':
      //call P + arm?
      drawTo(bx + 10 * scale, by);              // 10,0
      lift(0);
      drawTo(bx, by + 10 * scale);              // 0,10
      lift(0);
      goto drawP;
      break;
    case 's':
    case 'S':
      drawTo(bx, by);                           // 0,0
      lift(0);
      drawTo(bx + 10 * scale, by);              // 10,0
      drawTo(bx + 10 * scale, by + 10 * scale); // 10,10
      drawTo(bx, by + 10 * scale);              // 0,10
      drawTo(bx , by + 20 * scale);             // 0,20
      drawTo(bx + 10 * scale, by + 20 * scale); // 10,20
      lift(1);
      break;
    case 't':
      drawTo(bx , by + 20 * scale);             // 0,20
      lift(0);
      drawTo(bx + 10 * scale, by + 20 * scale); // 10,20
      lift(1);
      drawTo(bx + 5 , by + 20 * scale);         // 5,20
      lift(0);
      drawTo(bx + 5 , by + 0 * scale);          // 5,0
      lift(1);
      break;
    case 'u':
    case 'U':
drawU:
      drawTo(bx , by + 20 * scale);             // 0,20
      lift(0);
      drawTo(bx, by);                           // 0,0
      drawTo(bx + 10 * scale, by);              // 10,0
      drawTo(bx + 10 * scale, by + 20 * scale); // 10,20
      lift(1);
      break;
    case 'v':
    case 'V':
      drawTo(bx , by + 20 * scale);             // 0,20
      lift(0);
      drawTo(bx + 5, by);                       // 5,0
      drawTo(bx + 10 * scale, by + 20 * scale); // 10,20
      lift(1);
      break;
    case 'w':
    case 'W': // can also "just" do V V - a la ancient Rome where W was a VV 
      drawTo(bx + 5, by);                       // 5,0
      lift(0);
      drawTo(bx + 5, by + 3);                   // 5,3
      lift(1);
      goto drawU;
      break;
    case 'x':
    case 'X':
      drawTo(bx, by);                           // 0,0
      lift(0);
      drawTo(bx + 10 * scale, by + 20 * scale); // 10,20
      lift(1);
      drawTo(bx , by + 20 * scale);             // 0,20
      lift(0);
      drawTo(bx + 10 * scale, by);              // 10,0
      lift(1);
      break;
    case 'y':
    case 'Y':
      drawTo(bx, by);                           // 0,0
      lift(0);
      drawTo(bx + 10 * scale, by);              // 10,0
      drawTo(bx + 10 * scale, by + 20 * scale); // 10,20
      lift(1);
      drawTo(bx , by + 20 * scale);             // 0,20
      lift(0);
      drawTo(bx, by + 10 * scale);              // 0,10
      drawTo(bx + 10 * scale, by + 10 * scale); // 10,10
      lift(1);
      break;
    case 'z':
    case 'Z':
      drawTo(bx , by + 20 * scale);             // 0,20
      lift(0);
      drawTo(bx + 10 * scale, by + 20 * scale); // 10,20
      drawTo(bx, by);                           // 0,0
      drawTo(bx + 10 * scale, by);              // 10,0
      lift(1);
      break;
      default:
       // make whitespace if char is unknown
       
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
  c = floor(4 * sqrt(dx * dx + dy * dy));

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

String toString(int x){
  char snum[5];
  // convert 123 to string [buf]
  itoa(x, snum, 10);
  return snum;
}

String boolToString(bool x){
  return (x == false) ? "false" : "true";
  //return snum;
}

void set_XY(double Tx, double Ty) 
{
  Tx = (79-Tx);
  if(Tx < 5) Tx=5;

  //Serial.print("in set_XY  Tx=");
  //Serial.print(Tx);
  //Serial.print("  Ty=");
  //Serial.println(Ty);

  
  
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

  int ms = floor(((a2 + a1 - M_PI) * SERVOFAKTOR) + SERVOLEFTNULL);
  servo2.writeMicroseconds(ms);
  //Serial.print(" MicroSeconds  ");
  //Serial.println(ms);

  // calculate joinr arm point for triangle of the right servo arm
  a2 = return_angle(L2, L1, c);
  Hx = Tx + L3 * cos((a1 - a2 + 0.621) + M_PI); //36,5°
  Hy = Ty + L3 * sin((a1 - a2 + 0.621) + M_PI);

  // calculate triangle between pen joint, servoRight and arm joint
  dx = Hx - O2X;
  dy = Hy - O2Y;

  c = sqrt(dx * dx + dy * dy);
  a1 = atan2(dy, dx);
  a2 = return_angle(L1, (L2 - L3), c);

  servo3.writeMicroseconds(floor(((a1 - a2) * SERVOFAKTOR) + SERVORIGHTNULL));

}




int readEEPROM(int startByte) {

  byte high = EEPROM.read(startByte); //read the first half
  byte low = EEPROM.read(startByte + 1); //read the second half
  
  int xVal = (high << 8) + low; //reconstruct the integer
  return xVal;  
}

void writeEEPROM(int startByte, int xVal){
  
  EEPROM.write(startByte, highByte(xVal)); //write the first half
  EEPROM.write(startByte + 1, lowByte(xVal)); //write the second half

}

void writeDefaultEEPROM(){
    writeEEPROM(EEPROM_SERVOFAKTOR_ADDRESS, SERVOFAKTOR);
    writeEEPROM(EEPROM_LEFTNULL_ADDRESS, SERVOLEFTNULL);
    writeEEPROM(EEPROM_RIGHTNULL_ADDRESS, SERVORIGHTNULL);
    writeEEPROM(EEPROM_LIFT0_ADDRESS, LIFT0);
    writeEEPROM(EEPROM_LIFT1_ADDRESS, LIFT1);
    writeEEPROM(EEPROM_STATUS_ADDRESS, 123);   // magic number
}

