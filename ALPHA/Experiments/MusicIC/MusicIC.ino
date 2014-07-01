#include <SimpleSDAudio.h>

// Sound

int soundPin = 9;

// MISO MISO CS SCK are used as normal on the SPI port. CS is 10.

// Address pins and cs for comms with other processor

int add1 = A1;
int add2 = A2;
int add3 = A3; // Enough for 8 addresses
int play = A4; // All pulled high by internal pullup. Down is 'on'

byte *index; // index passed to receive_command function, determines track number.

// Our simple comms protocol should allow for 8-16 track selections.
char *AUDIO_FILES[] = {
  "ADMIN.AFM",
  "RELOAD.AFM",
  "SHOOT.AFM",
  "ALLOUT.AFM", 
  "HIT.AFM",
  "GRENADE.AFM",
  "POWERUP.AFM",
  "POWEROFF.AFM"
};

void setup() {

  pinMode(soundPin, OUTPUT); digitalWrite(soundPin, LOW);

  pinMode(add1, INPUT); digitalWrite(add1, HIGH);
  pinMode(add2, INPUT); digitalWrite(add2, HIGH);
  pinMode(add3, INPUT); digitalWrite(add3, HIGH);
  pinMode(play, INPUT); digitalWrite(play, HIGH);

  Serial.begin(9600);

  Serial.print(F("\nInitializing SD card..."));  
  
  SdPlay.setSDCSPin(10);
  
  if (!SdPlay.init(SSDA_MODE_FULLRATE | SSDA_MODE_MONO | SSDA_MODE_AUTOWORKER)) {
    Serial.println(F("initialization failed. Things to check:"));
    Serial.println(F("* is a card is inserted?"));
    Serial.println(F("* Is your wiring correct?"));
    Serial.println(F("* maybe you need to change the chipSelect pin to match your shield or module?"));
    Serial.print(F("Error code: "));
    Serial.println(SdPlay.getLastError());
    while(1);
  } 
  else {
    Serial.println(F("Wiring is correct and a card is present.")); 
  }
}


void loop(void) {

  if (receive_command(index)){
    if(!SdPlay.setFile(AUDIO_FILES[(*index)])) {
      Serial.println(F(" not found on card! Error code: "));
      Serial.println(SdPlay.getLastError());
      while(1);
    } else {
     Serial.println(F("Command received, playing...")); 
    }    

    SdPlay.play();
    while(!SdPlay.isStopped()) {
      ; // no worker needed anymore :-)
    }
    Serial.println(F("done."));
  }

}

boolean receive_command(byte *i){

  if (digitalRead(play)){
    return false;
  }
  else{
    byte temp = 0;
    temp = temp | (!digitalRead(add1));
    temp <<= 1;
    temp = temp | (!digitalRead(add2));
    temp <<= 1;
    temp = temp | (!digitalRead(add3));

    if ((temp >= 0) && (temp < 8)){
      *i = temp;
    }
    else {
      *i = 0;
    }
    
    return true;
  }
}