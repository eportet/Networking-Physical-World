#include <math.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>
SoftwareSerial XBee(2, 3); // RX, TX

int led_pin = 7;
int state = 0;
const int id = 123;
String pass = "";

String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void write(String pass, int address){
  if (pass.length() < 25) {
    EEPROM.put(address, pass.length());
    address += 2;
    for(int i = 0; i < pass.length(); i++) {
       EEPROM.write(address++, pass.charAt(i));
    }
  }
}

String read(int address) {
  int length = 0;
  EEPROM.get(address, length);
  String res = "";
  for(int i = address + 2; i < address + 2 + length; i++) {
    char c = char(EEPROM.read(i));
    Serial.println(c);
    res += c;
  }
  Serial.println(res);
  return res;
}

void setup() {
  // put your setup code here, to run once:
  pinMode(led_pin, OUTPUT);
  XBee.begin(9600);
  Serial.begin(9600);
  randomSeed(analogRead(0));
  digitalWrite(led_pin, state);
  boolean auth = false;

  int check = 0;
  EEPROM.get(500, check);
  if(check != 237) {
    String password = String(id);
    write(password, 0);
    pass = password;
    EEPROM.put(500, 237);
  }
  else {
    pass = read(0);
  }
  Serial.println("Password");
  Serial.println(read(0));
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
    if(getValue(message, ' ', 0) == "turn_on" && getValue(message, ' ' , 1) == String(id) && getValue(message, ' ', 2) == pass) {
      digitalWrite(led_pin, HIGH);
      state = 1;
    }
    if(getValue(message, ' ', 0) == "turn_off" && getValue(message, ' ' , 1) == String(id) && getValue(message, ' ', 2) == pass) {
      digitalWrite(led_pin, LOW);
      state = 0;
    }
    if(getValue(message, ' ', 0) == "toggle" && getValue(message, ' ' , 1) == String(id) && getValue(message, ' ', 2) == pass) {
      digitalWrite(led_pin, !state);
      state = !state;
    }
    if(getValue(message, ' ', 0) == "get" && getValue(message, ' ' , 1) == String(id) && getValue(message, ' ', 2) == pass) {
      String tempS = String(state);
      char chars[tempS.length()+1];
      tempS.toCharArray(chars, tempS.length()+1);
      XBee.write(chars);
      XBee.write("\n");
    }
    if(getValue(message, ' ', 0) == "set_pass" && getValue(message, ' ' , 1) == String(id) && getValue(message, ' ', 2) == pass) {
      write(getValue(message, ' ', 3), 0);
      pass = getValue(message, ' ', 3);
    }
    Serial.println(message);
  }
}
