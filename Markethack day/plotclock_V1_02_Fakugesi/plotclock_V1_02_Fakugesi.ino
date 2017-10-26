// Plotclock
// cc - by Johannes Heberlein 2014
// v 1.01
// thingiverse.com/joo   wiki.fablab-nuernberg.de

// units: mm; microseconds; radians
// origin: bottom left of drawing surface

// time library see http://playground.arduino.cc/Code/time 

// delete or mark the next line as comment when done with calibration  
//define CALIBRATION

// When in calibration mode, adjust the following factor until the servos move exactly 90 degrees
#define SERVOFAKTOR 680

// Zero-position of left and right servo
// When in calibration mode, adjust the NULL-values so that the servo arms are at all times parallel
// either to the X or Y axis
#define SERVOLEFTNULL 2100
#define SERVORIGHTNULL 1100

#define SERVOPINLIFT  16
#define SERVOPINLEFT  14
#define SERVOPINRIGHT 5

// lift positions of lifting servo
#define LIFT0 1500 // on drawing surface
#define LIFT1 1300  // between numbers
#define LIFT2 1200  // going towards sweeper

// speed of liftimg arm, higher is slower
#define LIFTSPEED 1500

// length of arms
#define L1 35
#define L2 55.1
#define L3 13.2


// origin points of left and right servo 
#define O1X 22 // was 22
#define O1Y -25
#define O2X 47 // was 47
#define O2Y -25



#include <Time.h> // see http://playground.arduino.cc/Code/time 
#include <Servo.h>

int servoLift = 1500;

Servo servo1;  // 
Servo servo2;  // 
Servo servo3;  // 

volatile double lastX = 75;
volatile double lastY = 47.5;

int last_min = 0;

void setup() 
{ 
  // Set current time only the first to values, hh,mm are needed
  setTime(19,38,0,0,0,0);

  drawTo(75.2, 47);
  lift(0);
  servo1.attach(SERVOPINLIFT);  //  lifting servo
  servo2.attach(SERVOPINLEFT);  //  left servo
  servo3.attach(SERVOPINRIGHT);  //  right servo
  delay(1000);

} 

void loop() 
{ 

#ifdef CALIBRATION

  Serial.println(F("Calibrarion Start"));
  // Servohorns will have 90° between movements, parallel to x and y axis
  drawTo(-3, 29.2);
  delay(500);
  drawTo(74.1, 28);
  delay(500);

  Serial.println(F("Calibrarion stop"));
#else 


  int i = 0;  
  if (last_min != minute()) {
    Serial.println(F("new minute"));

    if (!servo1.attached()) servo1.attach(SERVOPINLIFT);
    if (!servo2.attached()) servo2.attach(SERVOPINLEFT);
    if (!servo3.attached()) servo3.attach(SERVOPINRIGHT);

    lift(1); // stay up
    hour();
    while ((i+1)*10 <= hour())
    {
      i++;
    }

    //number(3, 3, 111, 1);  // wipe  ( 111 is Wipe motion )   
    lift(0);
    drawTo(0,0);
    drawTo(45,75);
     drawTo(0,75);
    drawTo(45,0);
    
    lift(0);// Get ready to write   
    number(0, 0, '0', 0.4);  // first character
     lift(1);
    number(10, 10, '0', 0.4);  // first character
     lift(1);
    number(20, 10, 'a', 0.4);  // first character
     lift(0);
    number(30, 10, '0', 0.4);  // first character
     lift(1);
    number(40, 10, 'a', 0.4);  // first character
     lift(0);
    number(50, 10, 'a', 0.4);  // first character
     lift(0);
    number(60, 10, '0', 0.4);  // first character
     lift(1);

 
    lift(1);

    delay(20000);


    

    servo1.detach();
    servo2.detach();
    servo3.detach();
  }

#endif

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

  case 12:
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

void set_XY(double Tx, double Ty) 
{
 // invert the X axis // hacky as hell
   Tx = (79-Tx) ;
   if(Tx<0)Tx=0;
  
  delay(2);
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
  a2 = return_angle(L1, (L2 - L3), c);

  servo3.writeMicroseconds(floor(((a1 - a2) * SERVOFAKTOR) + SERVORIGHTNULL));

}





