#include <Servo.h>

Servo wheels; // servo for turning the wheels
Servo esc; // not actually a servo, but controlled like one!
bool startup = true; // used to ensure startup only happens once
int startupDelay = 1000; // time to pause at each calibration step
double maxSpeedOffset = 45; // maximum speed magnitude, in servo 'degrees'
double maxWheelOffset = 85; // maximum wheel turn magnitude, in servo 'degrees'

void setup()
{
  wheels.attach(3); // initialize wheel servo to Digital IO Pin #8
  esc.attach(2); // initialize ESC to Digital IO Pin #9
  /*  If you're re-uploading code via USB while leaving the ESC powered on,
   *  you don't need to re-calibrate each time, and you can comment this part out.
   */

  pinMode(13, OUTPUT); //Right Trig
  pinMode(12, INPUT); //Right Echo
  pinMode(10, OUTPUT); //Left Trig
  pinMode(11, INPUT); //Left Echo
  pinMode(19, INPUT);

  calibrateESC();
  Serial.begin(9600);
  
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

/* Oscillate between various servo/ESC states, using a sine wave to gradually
 *  change speed and turn values.
 */
void oscillate(){
  for (int i =0; i < 360; i++){
    double rad = degToRad(i);
    double speedOffset = sin(rad) * maxSpeedOffset;
    double wheelOffset = sin(rad) * maxWheelOffset;
    esc.write(90 + speedOffset);
    wheels.write(90 + wheelOffset);
    delay(50);
  }

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

boolean off = false;
const float turn_constant = 45;

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
    esc.write(60);
    float right = distance(13,12);
    float left = distance(10,11);
    float delta = left - right;

    if(delta * delta > 400) {
      float timeToTurn = 0;
      float ratio = 0;
      if(left > right) {
        float adj = left / right;
        if(adj > 2)
          adj = 2;
        ratio = (adj - 1) * turn_constant;
        ratio *= -1;
      }
      else {
        float adj = right / left;
        if(adj > 2)
          adj = 2;
        ratio = (adj - 1) * turn_constant;
      }
      wheels.write(90 - ratio);
      delay(500);
      if(irDist() > 310.0) {
        return;
      }
      delay(500);
      wheels.write(90 + (ratio / 2.0));
      delay(500);
      wheels.write(90);
    }
  }
}
