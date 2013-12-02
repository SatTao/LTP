// This assumes a common cathode (common negative) config, so we need one resistor per bar.

//Pin connected to ST_CP of 74HC595
int latchPin = 5;
//Pin connected to SH_CP of 74HC595
int clockPin = 6;
////Pin connected to DS of 74HC595
int dataPin = 4;

volatile int fireFlag = LOW;
volatile int bother = LOW;

byte out = 0;

void setup() {
  Serial.begin(9600);
  
  //set pins to output because they are addressed in the main loop
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(2, INPUT);
  digitalWrite(2,HIGH);
  attachInterrupt(0,ISR_trigger,FALLING);
  reload();
  bother=HIGH;
}

void loop() {
  if (Serial.available()){
    char command = Serial.read();
    switch(command){
      case 'f': Serial.println("Firing"); fire(); break;
      default: Serial.println("No Code");
    }
  }
  if (fireFlag){
    fireFlag=LOW;
    Serial.println("Firing"); fire();
    
  }
} 

void ISR_trigger(){
  if (bother) {fireFlag=HIGH;bother=LOW;}
}

void fire(){
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, 0);   
  //return the latch pin high to signal chip that it 
  //no longer needs to listen for information
  digitalWrite(latchPin, HIGH);
  delay(500);
  reload();
}

void reload(){
  out =0;
  for (int i=1;i<10;i++){
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, out);   
  //return the latch pin high to signal chip that it 
  //no longer needs to listen for information
  digitalWrite(latchPin, HIGH);
  delay(i*40);
  out<<=1;
  out=out|1;
  }
  bother=HIGH;
}
  
