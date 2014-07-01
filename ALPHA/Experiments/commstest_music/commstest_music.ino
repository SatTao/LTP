// check the comms protocol works.

int add1 = A1;
int add2 = A2;
int add3 = A3;
int playPin = A4;

void setup(){
  delay(500);
  pinMode(add1, OUTPUT); digitalWrite(add1, HIGH);
  pinMode(add2, OUTPUT); digitalWrite(add2, HIGH);
  pinMode(add3, OUTPUT); digitalWrite(add3, HIGH);
  pinMode(playPin, OUTPUT); digitalWrite(playPin, HIGH); // High is inactive
}

void loop(){
  for (byte a = 0; a<8; a++){
    delay(5000); request_track(a);
  }
}

void request_track(byte track){
  byte out1 = track & 4;
  byte out2 = track & 2;
  byte out3 = track & 1;
  
  digitalWrite(add1, !out1);
  digitalWrite(add2, !out2);
  digitalWrite(add3, !out3);
  
  delay(5);
  
  digitalWrite(playPin, LOW);
  
  delay(20); // Give the musicIC long enough to register the playPin state
  
  digitalWrite(playPin, HIGH);
  digitalWrite(add1, HIGH);
  digitalWrite(add2, HIGH);
  digitalWrite(add3, HIGH);
}

  
  
