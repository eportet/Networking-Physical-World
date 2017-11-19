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

int leader = 0;

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

int s_count = 0;
int p_count = 0;

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
  if(leader == 0)
    id = random(15);
  else
    id = 15;

    Serial.println(id);
    
    predecessor_id = id;
    successor_id = id;
    String payload = String("IAM ") + String(id);
    char chars[payload.length() + 1];
    payload.toCharArray(chars, payload.length() + 1); 
    
    ZBTxRequest zbTx = ZBTxRequest(broadcast, chars, sizeof(chars));
    
    xbee.send(zbTx);
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
//    char payload[] = "hello there timmy";
//    ZBTxRequest zbTx = ZBTxRequest(broadcast, payload, sizeof(payload));
//    
//    xbee.send(zbTx);
  }
  prev_button = button;
}

void incrementId(int type) {

  if(leader == 0 && type == 1) {
    String payload = String("RESET");
      char chars[payload.length() + 1];
      payload.toCharArray(chars, payload.length() + 1); 
      ZBTxRequest zbTx = ZBTxRequest(broadcast, chars, sizeof(chars));
      xbee.send(zbTx);
      setupId();
  }
  else {
  Serial.println("Adding 1");
  if(successor_id == id && type == 1)
      successor_id = (successor_id + 1) % 16;
    if(predecessor_id == id  && type == 1)
      predecessor_id = (predecessor_id + 1) % 16;
    id = (id + 1) % 16;

    if(id <= successor_id) {
      Serial.println("Updating");
      Serial.println(id);
      String payload = String("UPDATE ") + String(id);
      char chars[payload.length() + 1];
      payload.toCharArray(chars, payload.length() + 1); 
      XBeeAddress64 addr1 = XBeeAddress64(successor_address);
      XBeeAddress64 addr2 = XBeeAddress64(predecessor_address);
      ZBTxRequest zbTx = ZBTxRequest(addr1, chars, sizeof(chars));
      xbee.send(zbTx);
      zbTx = ZBTxRequest(addr2, chars, sizeof(chars));
      xbee.send(zbTx);
    } else if(id == 0){
      
      
      String payload = String("BYE");
      char chars[payload.length() + 1];
      payload.toCharArray(chars, payload.length() + 1); 

      XBeeAddress64 addr1 = XBeeAddress64(predecessor_address);
      
      ZBTxRequest zbTx = ZBTxRequest(addr1, chars, sizeof(chars));
      xbee.send(zbTx);

      uint32_t low = predecessor_address % 0xFFFFFFFF; 
      uint32_t high = (predecessor_address >> 32) % 0xFFFFFFFF;

      payload = "IAM 0";
      char chars2[payload.length() + 1];
      payload.toCharArray(chars2, payload.length() + 1); 
      zbTx = ZBTxRequest(broadcast, chars2, sizeof(payload));
      xbee.send(zbTx);

    }
    }
}
int count = 0;
long startTime = 0;
void loop() {
  // put your main code here, to run repeatedly:
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
      delay(id * 30);
      setupId();
    }
    if(opcode == "IAM")
      handleIAM(message.data, message.address);
    else if(opcode == "IM") {
      handleIM(message.data, message.address);
    }
    else if(opcode == "BYE") {
      handleBye(message.data, message.address);
    } 
    else if(opcode == "LF") {
      handleLF(message.data, message.address);
    }
    else if(opcode == "PING") {
      handlePing(message.data, message.address);
    } else if(opcode == "UPDATE") {
      handleUpdate(message.data, message.address);
    }
  }

  int timeout = 100000;

  if(millis() - startTime > 4000 && successor_id != id) {
    Serial.println(startTime);
    Serial.println(millis());
     String payload = String("PING ");
      char chars[payload.length() + 1];
      payload.toCharArray(chars, payload.length() + 1); 
      XBeeAddress64 addr1 = XBeeAddress64(successor_address);
      ZBTxRequest zbTx = ZBTxRequest(broadcast, chars, sizeof(chars));
      xbee.send(zbTx);
      startTime = millis();
  }

  if(p_count == 0) {
    p_count = millis();
  }

  if(millis() - p_count > 10000 && leader == 0) {
      String payload = String("LF ") + String(predecessor_id);
      char chars[payload.length() + 1];
      payload.toCharArray(chars, payload.length() + 1); 
      
      ZBTxRequest zbTx = ZBTxRequest(broadcast, chars, sizeof(chars));
      xbee.send(zbTx);
      predecessor_id = id;
      p_count = millis();
  }
    
  if(predecessor_id == id)
    leader = 1;
  else
    leader = 0;

  count += 1;
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
    String payload = String("IM P ") + String(id);
    char chars[payload.length() + 1];
    payload.toCharArray(chars, payload.length() + 1); 
    XBeeAddress64 xbee_targ = XBeeAddress64(address);
    ZBTxRequest zbTx = ZBTxRequest(xbee_targ, chars, sizeof(chars));
    xbee.send(zbTx);
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
  Serial.println("IAMMING");
   int new_id = getValue(data, 1).toInt();
   Serial.print("evaluating ");
   Serial.print(new_id);
   Serial.print("   to ");
   Serial.print(id);
   Serial.print("\n");
   uint64_t target_address = address;
   uint32_t low = address % 0xFFFFFFFF; 
   uint32_t high = (address >> 32) % 0xFFFFFFFF;
   XBeeAddress64 xbee_targ = XBeeAddress64(address);
   if(id == successor_id) {
    String payload;
    Serial.print(" i runs");
      if(new_id > id) {
        successor_id = new_id;
        successor_address = target_address;
        Serial.print("P ");
        Serial.print(high, HEX);
        Serial.print(low, HEX);
        Serial.print("\n");
        payload = String("IM P ") + String(id);
      }
      else if(new_id < id) {
        predecessor_id = new_id;
      predecessor_address = target_address;
      payload = String("IM S ") + String(id);
      } else {
        
      }

      char chars[payload.length() + 1];
      payload.toCharArray(chars, payload.length() + 1); 
      
      ZBTxRequest zbTx = ZBTxRequest(xbee_targ, chars, sizeof(chars));
      xbee.send(zbTx);
   }
   else if(new_id > id && new_id <= successor_id) { //have new successor
      successor_id = new_id;
      successor_address = target_address;
      String payload = String("IM P ") + String(id);
      char chars[payload.length() + 1];
      payload.toCharArray(chars, payload.length() + 1); 
      ZBTxRequest zbTx = ZBTxRequest(xbee_targ, chars, sizeof(chars));
      xbee.send(zbTx);
   } else if((new_id > predecessor_id || predecessor_id == id) && new_id < id) { // new predecessor
      predecessor_id = new_id;
      predecessor_address = target_address;
      String payload = String("IM S ") + String(id);
      
      char chars[payload.length() + 1];
      payload.toCharArray(chars, payload.length() + 1); 
      ZBTxRequest zbTx = ZBTxRequest(xbee_targ, chars, sizeof(chars));
      xbee.send(zbTx);
   } else { //stole my identity (new predecessor / im the successor change my id)
      
      incrementId(0);

      predecessor_id = new_id;
      predecessor_address = target_address;
      String payload = String("IM S ") + String(id);
      
      char chars[payload.length() + 1];
      payload.toCharArray(chars, payload.length() + 1); 
      ZBTxRequest zbTx = ZBTxRequest(xbee_targ, chars, sizeof(chars));
      xbee.send(zbTx);
   }
}

struct Packet getDHTPacket() {
  Packet resp;
  resp.data = "";
  xbee.readPacket();
//  Serial.println("reading");
    if (xbee.getResponse().isAvailable()) {
      // got something
           
      if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE) {
        // got a zb rx packet
        
        // now fill our zb rx class
        xbee.getResponse().getZBRxResponse(rx);
      
//        Serial.println("Got an rx packet!");
            
        if (rx.getOption() == ZB_PACKET_ACKNOWLEDGED) {
            // the sender got an ACK
            Serial.println("packet acknowledged");
        } else {
          Serial.println("packet not acknowledged");
        }
        
//        Serial.print("checksum is ");
//        Serial.println(rx.getChecksum(), HEX);
//
//        Serial.print("packet length is ");
//        Serial.println(rx.getPacketLength(), DEC);

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

