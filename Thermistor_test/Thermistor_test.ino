#include <math.h>
#include <SoftwareSerial.h>
SoftwareSerial XBee(2, 3); // RX, TX

int AO = 0;
int id = -1;
void setup() {
  // put your setup code here, to run once:
  pinMode(AO, INPUT);
  XBee.begin(9600);
  Serial.begin(9600);
  randomSeed(analogRead(0));
}

void loop() {
  // put your main code here, to run repeatedly:

  if(id == -1) {
    String tempS = String(random(0, 1000));
    String looking = "GIVE " + tempS;
    tempS = "GET " + tempS;
    tempS += "\n";
    char chars[tempS.length() + 1];
    tempS.toCharArray(chars, tempS.length() + 1); 
    XBee.write(chars);

    Serial.println(chars);
    int counter = 0;
    while(id == -1) {
      String message = "";
      Serial.println(counter);
      while(true) {
        if(XBee.available() <= 0) {
          delay(100);
          continue;ls
          
        }
        char in = char(XBee.read());
        if (in == '\n')
          break;

        message += in;
      }
      Serial.println(message);
      if(message.substring(0, looking.length()) == looking) {
        String idS = message.substring(looking.length() + 1);
        id = idS.toInt();
        Serial.println("GOT ID");
        break;
      }
      else {
        counter += 1;
        if(counter > 100)
          return;
      }
    }
  }
  
  int voltage = analogRead(A0);
  double volts_across_therm = 5 - (5 * voltage / 1024.);

  double resistor_ohms = 10000;
  double resistance = volts_across_therm * resistor_ohms / (5 - volts_across_therm);
  float a = -14.6337;
  float b = 4971.842;
  float c = -115334;
  float d = -.000003730535;
  double a_1 = .003354016;
  double b_1 = .000256985;
  double c_1 = .000002620131;
  double d_1 = .00000006383091;
  double resistance_25 = 10000.0;
  double r = log(resistance/resistance_25);
  double temp =  1 / (a_1 + b_1 * r  + c_1 * r * r + d_1 * r * r * r);
  temp = temp - 273.15;
  String tempS = String(id) + " " + String(temp) + "\n";
  char chars[tempS.length()+1];
  tempS.toCharArray(chars, tempS.length()+1);
  Serial.println(chars);
  XBee.write(chars);
  Serial.println(temp);

  delay(2000);
}
