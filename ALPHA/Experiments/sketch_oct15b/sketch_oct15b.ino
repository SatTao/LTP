int in = 11;

void setup(){
  
  Serial.begin(9600);
  pinMode(in, INPUT);
}

void loop(){
  
  if (digitalRead(in)){
    Serial.println("0");
  }
  else{Serial.println("1");}
  
  delay(10);
}

