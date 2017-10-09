#include <math.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>
SoftwareSerial XBee(2, 3); // RX, TX

int led_pin = 7;
int state = 0;
String setPass = "SET PASS";

void setup() {
  // put your setup code here, to run once:
  pinMode(led_pin, OUTPUT);
  XBee.begin(9600);
  Serial.begin(9600);
  randomSeed(analogRead(0));
  digitalWrite(led_pin, state);

//  int check = 0;
//  EEPROM.get(500, check);
//  if(check == 237)
//    EEPROM.get(0, id);
//  Serial.println(id);
}

void loop() {
  // put your main code here, to run repeatedly:
  while(true) {
    String message = "";
    while(true) {
      if(XBee.available() <= 0) {
        delay(100);
        continue;
      }
      
      char in = char(XBee.read());
      if(in == '\n')
        break;
      message += char(in);
    }
    if(message == "turn on") {
      digitalWrite(led_pin, HIGH);
      state = 1;
    }
    if(message == "turn off") {
      digitalWrite(led_pin, LOW);
      state = 0;
    }
    if(message == "get") {
      String tempS = String(state);
      char chars[tempS.length()+1];
      tempS.toCharArray(chars, tempS.length()+1);
      XBee.write(chars);
      XBee.write("\n");
    }
    delay(1000);
    Serial.println(message);
  }
  
//  String tempS = String(id) + " " + String(temp);
//  char chars[tempS.length()+1];
//  tempS.toCharArray(chars, tempS.length()+1);
//  XBee.write(chars);
//  XBee.write("\n");
}
