#include <Printers.h>
#include <XBee.h>
#include <math.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>

XBee xbee = XBee(); // RX, TX
SoftwareSerial sSer(2,3);
ZBRxResponse rx = ZBRxResponse();

int id_pins[] = {4,5,6,7};
int num_pins = 4;

int leader = 1;

int leader_pin = 9;
int non_leader_pin = 8;

int input = 11;

int prev_button = 0;

int AO = 0;
XBeeAddress64 broadcast = XBeeAddress64(0x00000000, 0x0000FFFF);


int id = -1;
int predecessor_id = 0;
uint64_t predecessor_address = 0;
int successor_id = 0;
uint64_t successor_address = 0;

struct Packet {
  String data;
  uint64_t address;
};

void setup() {
  // put your setup code here, to run once:
  sSer.begin(9600);
  Serial.begin(9600);
  xbee.setSerial(sSer);
  randomSeed(analogRead(0));

  for(int i = 0; i < num_pins; i++) {
    pinMode(id_pins[i], OUTPUT);
  }
  pinMode(leader_pin, OUTPUT);
  pinMode(non_leader_pin, OUTPUT);
  pinMode(input, INPUT);

  setupId();   
}

long startTime = 0;
long p_sent = 0;
long p_received = 0;
long lp_received = 0;
long lonely_time = 0;

void loop() {
  
  displayAndHandleInput();

  while(true) {
    Packet message = getDHTPacket();
    if(message.data.length() == 0)
      break;
    uint32_t low = message.address % 0xFFFFFFFF;
    uint32_t high = (message.address >> 32) % 0xFFFFFFFF;
    Serial.print("Got \'");
    Serial.print(message.data);
    Serial.print("\' from ");
    Serial.print(high, HEX);
    Serial.print(low, HEX); 
    Serial.print("\n");

    String opcode = getValue(message.data, 0);
    if(opcode == "RESET") {
      if (leader == 1) {
        id = 15;
      } else {
        delay(id * 30);
        setupId(); 
      }
    } else if(opcode == "IAM") {
      handleIAM(message.data, message.address);
    } else if(opcode == "IM") {
      handleIM(message.data, message.address);
    } else if(opcode == "BYE") {
      handleBye(message.data, message.address);
    } else if(opcode == "LF") {
      handleLF(message.data, message.address);
    } else if(opcode == "PING") {
      handlePing(message.data, message.address);
    } else if(opcode == "UPDATE") {
      handleUpdate(message.data, message.address);
    }
  }

  int timeout = 5000;

  if(millis() - startTime > 2500 && successor_id != id && p_sent == 0) { //send ping
    Serial.println("PING");
    String payload = String("PING");
    XBeeAddress64 addr1 = XBeeAddress64(successor_address);
    sendPayload(payload, addr1);
    startTime = millis();
    p_sent = 1;
    p_received = 0;
  }

  else if(p_sent == 1 && p_received == 0 && millis() - startTime > timeout) { // if we have sent a ping and havent received an acknowledgment of the ping
    Serial.println("LOOKING FOR " + String(successor_id));
    String payload = String("LF ") + String(successor_id);
    sendPayload(payload, broadcast);
    Serial.println("IAM " + String(id));
    successor_id = id;
    predecessor_id = id;
    payload = String("IAM ") + String(id);
    sendPayload(payload, broadcast);
    p_sent = 0;
    p_received = 0;
  } else if(millis() - lp_received > timeout && lp_received != 0) { // if we have not received a ping in a while from our predecessor
    Serial.println("LOOKING FOR " + String(predecessor_id));
    String payload = String("LF ") + String(predecessor_id);
    sendPayload(payload, broadcast);
    Serial.println("IAM " + String(id));
    successor_id = id;
    predecessor_id = id;
    payload = String("IAM ") + String(id);
    sendPayload(payload, broadcast);
    p_sent = 0;
    p_received = 0;
    lp_received = 0;
  }

  if (successor_id == id && predecessor_id == id) {
    if (lonely_time == 0) {
      Serial.println("Im lonely");
      lonely_time = millis();
    } else if(millis() - lonely_time > timeout){
      Serial.println("I'll look for friends");
      String payload = String("IAM ") + String(id);
      sendPayload(payload,broadcast);
      lonely_time = 0;
    }
  }
  
    
  if(predecessor_id == id)
    leader = 1;
  else
    leader = 0;
}

String getValue(String data, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)== ' ' || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void setupId() {
  id = random(15);

  Serial.println(String("IAM ") + id);
  
  predecessor_id = id;
  successor_id = id;
  
  String payload = String("IAM ") + String(id);
  sendPayload(payload, broadcast);
}

void displayAndHandleInput() {
  int tmp = id;

  for(int i = 0; i < num_pins; i++) {
    digitalWrite(id_pins[i], tmp % 2);
    tmp /= 2;
  }
  if(leader != 0) {
    digitalWrite(leader_pin, HIGH);
    digitalWrite(non_leader_pin, LOW);
  }
  else {
    digitalWrite(leader_pin, LOW);
    digitalWrite(non_leader_pin, HIGH);
  }

  int button = digitalRead(input);

  if(button != prev_button && button == 0) {
    incrementId(1);
  }
  prev_button = button;
}

void incrementId(int type) {
  Serial.println("INCREMENTID");
  XBeeAddress64 addr1 = XBeeAddress64(successor_address);
  XBeeAddress64 addr2 = XBeeAddress64(predecessor_address);
  String payload;
  if (type == 1) {
    Serial.println("    BUTTON PRESSED");
    
    if (leader == 1 && successor_id != id) { // Leaders don't have predecessors
      if (id != 14) {
        id = (id + 1) % 16;
        Serial.print("       +1 leader ");
        Serial.println(id);
        predecessor_id = id;
        // Tell S that I incremented
        payload = String("UPDATE ") + String(id);
        sendPayload(payload, addr1);
      } else {
        id = 15;
        successor_id = id;
        payload = String("UPDATE ") + String(id);
        sendPayload(payload, addr1);
        payload = String("IAM ") + String(id);
        sendPayload(payload, broadcast);
      }
      
    } else if (leader == 1 && successor_id == id) {
      id = (id + 1) % 16;
      Serial.print("    Only one device ");
      Serial.println(id);
      predecessor_id = id;
      successor_id = id;
    
    } else if (leader == 0) { // Everyone rand id
      Serial.println("    RESET");
      payload = String("RESET");
      sendPayload(payload, broadcast);
      setupId(); // You have to reset as well since you dont get the message
    }
  } else { // My id was stolen
    Serial.println("    STOLEN ID");
    if (id != 15) {
      id = (id + 1) % 16;
      payload = String("UPDATE ") + String(id);
      sendPayload(payload, addr1);
      sendPayload(payload, addr2);
    } else {
      id = 0;
      leader = 0;
      //predecessor_id = id;
      payload = String("IAM ") + String(id);
      sendPayload(payload, broadcast);
    }
  }
}

void handleUpdate(String data, uint64_t address) {
  int new_id = getValue(data, 1).toInt();
  if(address == successor_address) {
    successor_id = new_id;
  } else if(address == predecessor_address) {
    predecessor_id = new_id;
  }

  if(new_id == id) {
    incrementId(0);
  }
}

void handleLF(String data, uint64_t address) {
  int old_id = getValue(data, 1).toInt();
  if(old_id == predecessor_id || old_id == successor_id) {
    XBeeAddress64 xbee_targ = XBeeAddress64(address);
    String payload = String("IAM ") + String(id);
    sendPayload(payload, xbee_targ);
  }
}

void handlePing(String data, uint64_t address) {
  if(address == predecessor_address) {
    lp_received = millis();
    XBeeAddress64 xbee_targ = XBeeAddress64(address);
    String payload = String("IM S ") + String(id);
    sendPayload(payload, xbee_targ);
  }
}

void handleBye(String data, uint64_t address) {
  successor_id = id;
}

void handleIM(String data, uint64_t address) {
  String type = getValue(data, 1);
  uint64_t target_address = address;
  int new_id = getValue(data, 2).toInt();
  if(type == "P") {
    predecessor_id = new_id;
    predecessor_address = target_address;
  } else if(type == "S") {
    p_received = 1;
    p_sent = 0;
    successor_id = new_id;
    successor_address = target_address;
  } else {
    successor_id = new_id;
    successor_address = target_address;
    predecessor_id = new_id;
    predecessor_address = target_address;
  }
}

void handleIAM(String data, uint64_t address) {
  Serial.print("IAMMING ");
  int new_id = getValue(data, 1).toInt();
  Serial.print(new_id);
  Serial.print(" into ");
  Serial.println(id);
  
  uint64_t target_address = address;
  uint32_t low = address % 0xFFFFFFFF; 
  uint32_t high = (address >> 32) % 0xFFFFFFFF;
  XBeeAddress64 xbee_targ = XBeeAddress64(address);
  String payload;

  if (new_id == id) { // stolen id
    Serial.print("    STOLEN ID");
    incrementId(0);
    Serial.print("    YOU ARE P");
    Serial.print("    I AM S");
    predecessor_id = new_id;
    predecessor_address = target_address;
    payload = String("IM S ") + String(id);
  } else if (new_id > id) { // potential successor
    if (new_id <= successor_id || id == successor_id) { // change successor
      Serial.println("    YOU ARE S");
      Serial.println("    IM P");
      successor_id = new_id;
      successor_address = target_address;
      payload = String("IM P ") + String(id);
    }
    // dont change anything
  } else if (new_id < id) { // potential predecessor
    if (new_id >= predecessor_id || id == predecessor_id) { // change predecessor
       Serial.println("    YOU ARE P");
       Serial.println("    IM S");
       predecessor_id = new_id;
       predecessor_address = target_address;
       payload = String("IM S ") + String(id);
     }
     // dont change anything
  }
  sendPayload(payload, xbee_targ);
}

void sendPayload(String payload, XBeeAddress64 target) {
  char chars[payload.length() + 1];
  payload.toCharArray(chars, payload.length() + 1); 
  ZBTxRequest zbTx = ZBTxRequest(target, chars, sizeof(chars));
  xbee.send(zbTx);
}

struct Packet getDHTPacket() {
  Packet resp;
  xbee.readPacket();
  if (xbee.getResponse().isAvailable()) { // got something     
    if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE) { // got a zb rx packet
      // now fill our zb rx class
      xbee.getResponse().getZBRxResponse(rx);            
      if (rx.getOption() == ZB_PACKET_ACKNOWLEDGED) { // the sender got an ACK
      } else {
        //Serial.println("packet not acknowledged");
      }
  
      resp.data = "";
      for (int i = 0; i < rx.getDataLength(); i++) {
        resp.data += char(rx.getData()[i]);
      }
      
      resp.address = 0;
      for (int i = 0; i < 8; i++) {
        uint64_t temp = xbee.getResponse().getFrameData()[i];
        resp.address |= (uint64_t) (temp << (8 * (7 - i)));
      }
  
    }
  } else if (xbee.getResponse().isError()) {
    Serial.print("error code:");
    Serial.println(xbee.getResponse().getErrorCode());
  }
  return resp;
}

