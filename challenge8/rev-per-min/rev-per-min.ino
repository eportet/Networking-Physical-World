long int t = millis();
long int t1 = millis();
float rpm = 0;

const int numReadings = 24;
int readings[numReadings];      // the readings from the analog input
int readIndex = 0;              // the index of the current reading
int total = 0;                  // the running total
int average = 0;                // the average

void setup() {
    pinMode(2, INPUT);
    attachInterrupt(0, doEncoder, CHANGE);
    Serial.begin(9600);
    for (int thisReading = 0; thisReading < numReadings; thisReading++) {
      readings[thisReading] = 0;
    }
}

void loop() {
  // subtract the last reading:
  total = total - readings[readIndex];
  // read from the sensor:
  readings[readIndex] = rpm;
  // add the reading to the total:
  total = total + readings[readIndex];
  // advance to the next position in the array:
  readIndex = readIndex + 1;

  // if we're at the end of the array...
  if (readIndex >= numReadings) {
    // ...wrap around to the beginning:
    readIndex = 0;
  }

  // calculate the average:
  average = total / numReadings;
  // send it to the computer as ASCII digits
  delay(1);
  if (millis() - t1 > 2000) {
    t1 = millis();
    Serial.println(average);
  }
}

void doEncoder() {
  if ((millis()-t) > 1500){
    rpm = 0;
    t = millis();
  } else {
    rpm = ((1.0/24.0)*(60000))/(millis()-t);
    t = millis();
    //Serial.println(rpm);
  }
}

