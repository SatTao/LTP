#include <IRremote.h>

IRsend irsend;

int k_device = 0;
int k_command = 0;
int k_state = 0; // 0 is empty, 1 is device entry, 2 is command entry, 3 is ready/fired
int k_entry = 0;

#include <Keypad.h>

const byte ROWS = 4;
const byte COLS = 4;
int led = 13;
int ledstat = 0;

char keys[ROWS][COLS] = {
  {'1','2','3','U'},
  {'4','5','6','D'},
  {'7','8','9','M'},
  {'A','0','B','E'}
};

byte rowPins[ROWS] = {12, 11, 10, 9};
byte colPins[COLS] = {8, 7, 6, 5};

Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS);

void setup()
{
  Serial.begin(9600);
  
}

unsigned long encodeSony(byte device, byte command){
  
  if ((device < 1 or device > 31) or (command < 1 or command > 127)){ // check they're within bounds
    Serial.println("Impossible device or command code");
    return 0;
  }
  
  // Reverse the command code
  
  byte crev = 0;
  for(int i=0;i<7;i++){
    crev <<= 1;
    crev += command & 1;
    command >>= 1;
  }

  // Reverse the device code
  
  byte drev = 0;
  for(int i=0;i<5;i++){
    drev <<= 1;
    drev += device & 1;
    device >>= 1;
  }

  // Concatenate into unsigned long
  
  unsigned long result = crev; // results in leading zeros.
  result = (result << 5) | drev;
  
  return result;
}

void PrintDetailsOfSony(unsigned long input){
  
  // input is a long number from the IRremote library. At the far end of it is the device code.

  unsigned long dinput = input << 27;
  dinput >>= 27; // Isolates the device code on the LSB side: 000........ddddd
  
  unsigned long cinput = input >> 5;
  cinput <<= 25;
  cinput >>= 25; // Isolates the command code on the LSB side: 000........ccccccc
  
  
  // Reverse the device code
  byte drev = 0;
  for(int i=0;i<5;i++){
    drev <<= 1;
    drev += dinput & 1;
    dinput >>= 1;
  }
  
  // Reverse the command code
  byte crev = 0;
  for(int i=0;i<7;i++){
    crev <<= 1;
    crev += cinput & 1;
    cinput >>= 1;
  }
  
  // print them
  
  Serial.print("Device code:\t"); Serial.println(drev);
  Serial.print("Command code:\t"); Serial.println(crev);
}

void docode(){
  if (k_device != 0 && k_command != 0 && k_state == 3){
    unsigned long r = encodeSony(k_device, k_command);
    for (int i = 0; i < 3; i++) {
      irsend.sendSony(r, 12); // Sony TV power code
      delay(40);
    }
    //PrintDetailsOfSony(r);
  }
}

void clearcodes(){
  k_device=0;
  k_command=0;
  k_state = 0;
  k_entry = 0;
}

void advancestate(){
  if(k_state < 3){
    if (k_state == 1){k_device = k_entry;}
    if (k_state == 2){k_command = k_entry;}
    k_state += 1; k_entry = 0;}
}

void numin(int key){
  
  if(k_state==0){advancestate();}
  
  if(k_entry==0) {// Fresh start
    k_entry = key;
  }
  else{
    k_entry *= 10;
    k_entry += key;
  }
}

void loop() {
  
  char key = kpd.getKey();
  
  if (key!=NO_KEY){
    switch(key){
      case 'A': Serial.println("\tADD"); advancestate(); break;
      case 'U': Serial.println("UP");; break;
      case 'E': Serial.println("\tENTER"); docode(); break;
      case 'D': Serial.println("DOWN"); break;
      case 'M': Serial.println("MENU"); break;
      case 'B': Serial.println("BACK"); clearcodes(); break;
      default: Serial.print(key); numin(atoi(&key));
    }
  }
  
  
}

// Useless change for git
