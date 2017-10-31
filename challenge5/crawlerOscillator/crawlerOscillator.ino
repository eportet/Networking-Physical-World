#include <SonarEZ0pw.h>

#include "Arduino.h"
#include "PID_v1.h"
#include <Servo.h>

Servo wheels; // servo for turning the wheels
Servo esc; // not actually a servo, but controlled like one!
bool startup = true; // used to ensure startup only happens once
int startupDelay = 1000; // time to pause at each calibration step
double maxSpeedOffset = 45; // maximum speed magnitude, in servo 'degrees'
double maxWheelOffset = 85; // maximum wheel turn magnitude, in servo 'degrees'

//Define Variables we'll be connecting to
double setpoint, pid_input, pid_output;
PID myPID(&pid_input, &pid_output, &setpoint, .5, .5, .5, 0); //P_ON_M specifies that Proportional on Measurement be used
                                                            //P_ON_E (Proportional on Error) is the default behavior
SonarEZ0pw sonarLeft(6); // pin D7
SonarEZ0pw sonarRight(5); // pin D7

float prev_left = -1;
float prev_right = -1;

void setup()
{
  Serial.begin(9600);
  Serial.println("hi");
  wheels.attach(2); // initialize wheel servo to Digital IO Pin #8
  esc.attach(3); // initialize ESC to Digital IO Pin #9
//  
//  myPID.SetSampleTime(50);
  //myPID.SetOutputLimits(-45, 45);
  //myPID.SetControllerDirection(REVERSE);
  setpoint = 255;
  pinMode(19, INPUT);

  Serial.println("Here");

  int count = 0;
  float s1 = 0;
  float s2 = 0;
  while(count < 2 * 20) {
    Serial.println(count);
    s1 += sonarLeft.Distance(cm);
    s2 += sonarRight.Distance(cm);
    count += 1;
    Serial.println(count);
  }

  prev_left = s1 / 40;
  prev_right = s2 / 40;
  Serial.println(prev_left);
  Serial.println(prev_right);
  
  calibrateESC();
  esc.write(55);

  myPID.SetMode(AUTOMATIC);
}

/* Convert degree value to radians */
double degToRad(double degrees){
  return (degrees * 71) / 4068;
}

/* Convert radian value to degrees */
double radToDeg(double radians){
  return (radians * 4068) / 71;
}

/* Calibrate the ESC by sending a high signal, then a low, then middle.*/
void calibrateESC(){
    esc.write(180); // full backwards
    delay(startupDelay);
    esc.write(0); // full forwards
    delay(startupDelay);
    esc.write(90); // neutral
    delay(startupDelay);
    esc.write(90); // reset the ESC to neutral (non-moving) value
}

float distance(int trig, int echo) {
  long duration, distanceCm, distanceIn;
 
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  duration = pulseIn(echo, HIGH);
 
  // convert the time into a distance
  return duration / 29.1 / 2 ;
}

float irDist() {
  return analogRead(19);
}

float getMedian(float a, float b, float c) {
    float x = a-b;
    float y = b-c;
    float z = a-c;
    if(x*y > 0) return b;
    if(x*z > 0) return c;
    return a;
}

boolean off = false;
const float turn_constant = 45;

float lefts[3] =  {0,0,0};
float rights[3] = {0,0,0};


void loop()
{
  if(off) {
    delay(1000);
    esc.write(90);
    return;
  }
  float frontDistance = irDist();
  if(irDist() > 310.0) {
    Serial.println("STOPPED");
    esc.write(90);
    off = true;
  }
  else { //doStuff
   // float reads[3] = {0,0,0};
//    for(int i = 0; i < 3; i++) {
//      reads[i] = distance(13,12);
//    }
  //  float right = distance(13,12);//getMedian(reads[0], reads[1], reads[2]);

//    for(int i = 0; i < 3; i++) {
//      reads[i] = distance(10, 11);
//    }
   // float left = distance(10, 11);//getMedian(reads[0], reads[1], reads[2]);
    float left = sonarLeft.Distance(cm);
    float right = sonarRight.Distance(cm);

    if((left - prev_left) * (left - prev_left) < 12 * 12) {
      prev_left = left;
    } else {
      left = prev_left;
    }

    if((right - prev_right) * (right - prev_right) < 12 * 12) {
      prev_right = right;
    } else {
      right = prev_right;
    }

    for(int i = 1; i < 3; i++) {
      rights[i] = rights[i-1];
      lefts[i] = lefts[i-1];
    }
    
    lefts[0] = left;
    rights[0] = right;

    left = (lefts[0] + lefts[1] + lefts[2]) / 3;
    right = (rights[0] + rights[1] + rights[2]) / 3;
    
    float delta = left - right;
    float mul = 1;
    if(delta < 0) {
      delta *= -1;
      mul = -1;
    }
    
    if(delta > 100)
      delta = 100;

    delta = 255.0 * delta / 100.0;
  
    delta = 255 - delta;
    pid_input = delta;
    myPID.Compute();
    
    Serial.println(String("left ") + String(left));
    Serial.println(String("right ") + String(right));
    Serial.println(String("PID Out ") + pid_output);
    Serial.println(String("write ") + (90+pid_output / 255 * mul * 50));
    Serial.println(String("---------------"));
    wheels.write(90 + (pid_output / 255.0 * mul * 25));
    
    delay(40);
  }
}
