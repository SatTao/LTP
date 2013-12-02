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

byte rowPins[ROWS] = {9,8,7,6};
byte colPins[COLS] = {5,4,3,2};

Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS);

void setup(){
  Serial.begin(9600);
  pinMode(13,OUTPUT);
  digitalWrite(13,LOW);
}
void toggle(){
  if (ledstat==0){
    digitalWrite(led,HIGH);
    ledstat=1;
  }
  else{
    digitalWrite(led,LOW);
    ledstat=0;
  }
}
void loop(){
  char key = kpd.getKey();
  
  if (key!=NO_KEY){
    switch(key){
      case 'A': Serial.println("ADD"); break;
      case 'U': Serial.println("UP");; break;
      case 'E': Serial.println("ENTER"); toggle(); break;
      case 'D': Serial.println("DOWN"); break;
      case 'M': Serial.println("MENU"); break;
      case 'B': Serial.println("BACK"); break;
      default: Serial.println(key);
    }
  }
}

