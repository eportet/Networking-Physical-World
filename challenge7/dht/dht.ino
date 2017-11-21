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

int ALIVE = 0;

int leader_pin = 9;
int non_leader_pin = 8;

int input = 11;

int prev_button = 0;

int AO = 0;
int id = -1;
XBeeAddress64 broadcast = XBeeAddress64(0x00000000, 0x0000FFFF);

int predecessor_id = 0;
int successor_id = 0;

uint64_t successor_address = 0;
uint64_t predecessor_address = 0;

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
      predecessor_id = id;
      payload = String("IAM ") + String(id);
      sendPayload(payload, broadcast);
    }
  }
  
  
  
  /*
  if(leader == 0 && type == 1) {
    String payload = String("RESET");
    sendPayload(payload, broadcast);
    setupId(); // You have to reset as well since you dont get the message
  } else {
    Serial.println("    Adding 1");
    if(successor_id == id && type == 1)
        successor_id = (successor_id + 1) % 16;
    if(predecessor_id == id  && type == 1)
        predecessor_id = (predecessor_id + 1) % 16;
    id = (id + 1) % 16;
  
    if(id <= successor_id) {
      Serial.print("    Updating ");
      Serial.println(id);
      
      XBeeAddress64 addr1 = XBeeAddress64(successor_address);
      XBeeAddress64 addr2 = XBeeAddress64(predecessor_address);
      String payload = String("UPDATE ") + String(id);
      sendPayload(payload, addr1);
      sendPayload(payload, addr2);
    } else if(id == 0) {
      XBeeAddress64 addr1 = XBeeAddress64(predecessor_address);
      String payload = String("BYE");      
      sendPayload(payload, addr1);
      
      uint32_t low = predecessor_address % 0xFFFFFFFF; 
      uint32_t high = (predecessor_address >> 32) % 0xFFFFFFFF;
      
      payload = "IAM 0";
      sendPayload(payload, broadcast);
    }
  }*/
}

int p_count = 0;
long startTime = 0;

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

  /*
  int timeout = 10000;

  if(millis() - startTime > timeout && successor_id != id) {
    //Serial.println(startTime);
    //Serial.println(millis());
    String payload = String("PING ");
    XBeeAddress64 addr1 = XBeeAddress64(successor_address);
    sendPayload(payload, addr1);
    startTime = millis();
  }

  if(p_count == 0) {
    p_count = millis();
  }

  if(millis() - p_count > timeout && leader == 0) {
      String payload = String("LF ") + String(predecessor_id);
      sendPayload(payload, broadcast);
      predecessor_id = id;
      p_count = millis();
  }*/
    
  if(predecessor_id == id)
    leader = 1;
  else
    leader = 0;
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
  if(old_id == successor_id) {
    successor_id = getValue(data, 2).toInt();
    successor_address = address;

    XBeeAddress64 xbee_targ = XBeeAddress64(address);
    String payload = String("IM P ") + String(id);
    sendPayload(payload, xbee_targ);
  }
}

void handlePing(String data, uint64_t address) {
  if(address == predecessor_address)
    p_count = millis();
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
  
  /*if(id == successor_id) {
    Serial.println("    IM MY OWN SUCCESSOR");
    if(new_id > id) { // we get a new successor / become predecesor
      successor_id = new_id;
      successor_address = target_address;
      Serial.print("        newid > id IM P ");
      Serial.println(id);
      payload = String("IM P ") + String(id);
    } else if(new_id < id) {
      Serial.print("        newid < id IM P ");
      predecessor_id = new_id;
      predecessor_address = target_address;
      payload = String("IM S ") + String(id);
    }
    sendPayload(payload, xbee_targ);
  } else if(new_id > id && new_id <= successor_id) { //have new successor
      successor_id = new_id;
      successor_address = target_address;
      String payload = String("IM P ") + String(id);
      sendPayload(payload, xbee_targ);
   } else if((new_id > predecessor_id || predecessor_id == id) && new_id < id) { // new predecessor
      predecessor_id = new_id;
      predecessor_address = target_address;
      String payload = String("IM S ") + String(id);
      sendPayload(payload, xbee_targ);
   } else { //stole my identity (new predecessor / im the successor change my id)
      
      incrementId(0);

      predecessor_id = new_id;
      predecessor_address = target_address;
      String payload = String("IM S ") + String(id);
      
      sendPayload(payload, xbee_targ);
   }*/
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
        Serial.println("    ACK");
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

