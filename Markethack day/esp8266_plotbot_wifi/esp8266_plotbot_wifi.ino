/**
  BasicHTTPClient.ino

   Created on: 24.05.2015

   4MB flash :D

   (1) use "Wifimanager" library for esp8266 to make it easy for the user to make it join their wifi
   (2) set initial RTC time to compile time

**/

// delete or mark the next line as comment if you don't need these
#define CALIBRATION      // enable calibration mode
// #define REALTIMECLOCK    // enable real time clock

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>


#include <Time.h> // see http://playground.arduino.cc/Code/time 
#include <Servo.h>

#ifdef REALTIMECLOCK
// for instructions on how to hook up a real time clock,
// see here -> http://www.pjrc.com/teensy/td_libs_DS1307RTC.html
// DS1307RTC works with the DS1307, DS1337 and DS3231 real time clock chips.
// Please run the SetTime example to initialize the time on new RTC chips and begin running.

#include <Wire.h>
#include <DS1307RTC.h> // see http://playground.arduino.cc/Code/time    
#endif




// When in calibration mode, adjust the following factor until the servos move exactly 90 degrees
#define SERVOFAKTORLEFT 670
#define SERVOFAKTORRIGHT 670

// Zero-position of left and right servo
// When in calibration mode, adjust the NULL-values so that the servo arms are at all times parallel
// either to the X or Y axis
#define SERVOLEFTNULL 2200
#define SERVORIGHTNULL 1050

// SERVO PINS
#define SERVOPINLIFT  16
#define SERVOPINLEFT  14
#define SERVOPINRIGHT 5

// lift positions of lifting servo
#define LIFT0 1550 // on drawing surface
#define LIFT1 1300  // between numbers
#define LIFT2 1200  // going towards sweeper

// speed of liftimg arm, higher is slower
#define LIFTSPEED 1500

// length of arms
#define L1 35
#define L2 55.1
#define L3 13.2

// origin points of left and right servo
#define O1X 22
#define O1Y -25
#define O2X 47
#define O2Y -25

#define USE_SERIAL Serial

ESP8266WiFiMulti WiFiMulti;


// How many times to try and connect to wifi
#define WIFI_MAX_ATTEMPTS 2
#define WIFI_SSID "ContentBarBase"
#define WIFI_PASSWORD "myName1sGru"

#define RUN_MODE_TWITTER 0
#define RUN_MODE_CLOCK 1

String PLOTBOT_ID = "1";
String SERVICE_URL = "http://www.dinofizzotti.com/plotbots/" + PLOTBOT_ID + "/";
String tweet_text;

#define GREEN_LED 12
#define RED_LED 15
#define BLUE_LED 13


// 0 - twitter
// 1 - clock
uint8_t run_mode;
uint8_t connection_count;

// TX = GPIO1 = SCL
// RX = GPIO3 = SDA
#define SCL_PIN 1
#define SDA_PIN 3


int servoLift = 1500;

Servo servo1;  //
Servo servo2;  //
Servo servo3;  //

volatile double lastX = 75;
volatile double lastY = 47.5;

int last_min = 0;



void drawTo(double pX, double pY);
void bogenUZS(float bx, float by, float radius, int start, int ende, float sqee);
void bogenGZS(float bx, float by, float radius, int start, int ende, float sqee);

void loop() {

#ifdef CALIBRATION

  // Servohorns will have 90° between movements, parallel to x and y axis
  drawTo(-3, 29.2);
  delay(500);
  drawTo(74.1, 28);
  delay(500);

#else

  // Wait for WiFi connection, clock mode if not available.
  if (WiFiMulti.run() != WL_CONNECTED && run_mode == RUN_MODE_TWITTER)
  {
    blink(BLUE_LED, 3, 200);
    if (connection_count > WIFI_MAX_ATTEMPTS) {
      run_mode = RUN_MODE_CLOCK;
      blink(RED_LED, 3, 100);
      connection_count++;
    }
  }

  if (run_mode == RUN_MODE_TWITTER) {

    //sanity check.
    if ((WiFiMulti.run() == WL_CONNECTED)) {
      get_twitter_chars();

    }

  } else {
    plot_time();
  }

  delay(10000);
#endif
}


void setup() {

  //Wire.begin(SDA_PIN, SCL_PIN);

#ifdef USE_SERIAL
  USE_SERIAL.begin(115200);
  // USE_SERIAL.setDebugOutput(true);

  USE_SERIAL.println();
  USE_SERIAL.println();
  USE_SERIAL.println();
#endif

  for (uint8_t t = 4; t > 0; t--) {
#ifdef USE_SERIAL
    USE_SERIAL.printf("[SETUP] WAIT %d...\n");
    USE_SERIAL.flush();
#endif
    delay(1000);
  }

  WiFiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);

  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);

  tweet_text = "";


#ifdef REALTIMECLOCK
  tmElements_t tm;
  if (RTC.read(tm))
  {
    setTime(tm.Hour, tm.Minute, tm.Second, tm.Day, tm.Month, tm.Year);
    blink(GREEN_LED, 5, 100);
    //USE_SERIAL.println("DS1307 time is set OK.");
  }
  else
  {
    if (RTC.chipPresent())
    {
      blink(RED_LED, 5, 100);
      //USE_SERIAL.println("DS1307 is stopped.  Please run the SetTime example to initialize the time and begin running.");
    }
    else
    {
      blink(RED_LED, 5, 100);
      //USE_SERIAL.println("DS1307 read error!  Please check the circuitry.");
    }
    // Set current time only the first to values, hh,mm are needed
    setTime(19, 38, 0, 0, 0, 0);
  }
#else
  // Set current time only the first to values, hh,mm are needed
  setTime(19, 38, 0, 0, 0, 0);
#endif

  drawTo(75.2, 47);
  lift(0);
  servo1.attach(SERVOPINLIFT);  //  lifting servo
  servo2.attach(SERVOPINLEFT);  //  left servo
  servo3.attach(SERVOPINRIGHT);  //  right servo
  delay(1000);

  run_mode = RUN_MODE_TWITTER;
  connection_count = 0;
}

void blink(char pin, int loopCount, int delayMilliseconds) {

  for (int i = 0; i < loopCount; i++) {
    digitalWrite(pin, HIGH);
    delay(delayMilliseconds);
    digitalWrite(pin, LOW);
    delay(delayMilliseconds);
  }

}

String get_twitter_chars() {

  blink(GREEN_LED, 2, 200);
  String payload = "";
  HTTPClient http;

#ifdef USE_SERIAL
  USE_SERIAL.print("[HTTP] begin...\n");
#endif
  http.begin(SERVICE_URL); //HTTP

#ifdef USE_SERIAL
  USE_SERIAL.print("[HTTP] GET...\n");
#endif
  // start connection and send HTTP header
  int httpCode = http.GET();

  // httpCode will be negative on error
  if (httpCode > 0) {
    // HTTP header has been send and Server response header has been handled
    USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);

    // file found at server
    if (httpCode == HTTP_CODE_OK) {
      payload = http.getString();
      USE_SERIAL.println(payload);
      blink(BLUE_LED, 2, 200);
    }
  } else {
    USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    blink(RED_LED, 2, 200);
  }
  http.end();
  return payload;
}

void plot_time() {
  // Plot the time.
  int i = 0;
  if (last_min != minute()) {
    if (!servo1.attached()) servo1.attach(SERVOPINLIFT);
    if (!servo2.attached()) servo2.attach(SERVOPINLEFT);
    if (!servo3.attached()) servo3.attach(SERVOPINRIGHT);

    lift(0);

    hour();
    while ((i + 1) * 10 <= hour())
    {
      i++;
    }

    // old code calling 'number()'
    /* number(3, 3, 111, 1); */
    /* number(5, 25, i, 0.9); */
    /* number(19, 25, (hour() - i * 10), 0.9); */
    /* number(28, 25, 11, 0.9); */

    // Cast things to char's':
    char x = (char) i;
    char y = (char) (hour() - i * 10);

    draw_char(3, 3, '/', 1);
    draw_char(5, 25, x, 0.9);
    draw_char(19, 25, y, 0.9);
    draw_char(28, 25, '?', 0.9);

    i = 0;
    while ((i + 1) * 10 <= minute())
    {
      i++;
    }
    //number(34, 25, i, 0.9);
    //number(48, 25, (minute() - i * 10), 0.9);

    x = (char) i;
    y = (char) (minute() - i * 10);
    draw_char(34, 25, x, 0.9);
    draw_char(48, 25, y, 0.9);


    lift(2);
    drawTo(74.2, 47.5);
    lift(1);
    last_min = minute();

    servo1.detach();
    servo2.detach();
    servo3.detach();
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


// Writing numeral with bx by being the bottom left originpoint. Scale 1 equals a 20 mm high font.
// The structure follows this principle: move to first startpoint of the numeral, lift down, draw numeral, lift up
void draw_char(float bx, float by, char c, float scale) {
  switch (c) {
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

    case '/':
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
    case 'o':
    case 'O':
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

void set_XY(double Tx, double Ty)
{
  
   // invert the X axis // hacky as hell - The 79 was "imperically" determined
   Tx = (79-Tx) ;
   if(Tx<0)Tx=0;
  
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

  servo2.writeMicroseconds(floor(((a2 + a1 - M_PI) * SERVOFAKTORLEFT) + SERVOLEFTNULL));

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

  servo3.writeMicroseconds(floor(((a1 - a2) * SERVOFAKTORRIGHT) + SERVORIGHTNULL));

}
