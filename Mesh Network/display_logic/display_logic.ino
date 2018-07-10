
int id = 5;
int id_pins[] = {2,3,4,5};
int num_pins = 4;

int leader = 0;

int leader_pin = 9;
int non_leader_pin = 8;

int input = 11;

int prev_button = 0;

void setup() {
  for(int i = 0; i < num_pins; i++) {
    pinMode(id_pins[i], OUTPUT);
  }
  pinMode(leader_pin, OUTPUT);
  pinMode(non_leader_pin, OUTPUT);
  pinMode(input, INPUT);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  //id++;

  int tmp = id;

  for(int i = 0; i < num_pins; i++) {
    digitalWrite(id_pins[i], tmp % 2);
    tmp /= 2;
  }

  digitalWrite(leader_pin, leader);
  digitalWrite(non_leader_pin, ~leader);

  int button = digitalRead(input);

  if(button != prev_button && button == 0) {
    id++;
  }
  prev_button = button;
  Serial.println(String(button) + " " + String(id));


}
