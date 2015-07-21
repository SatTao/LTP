#include <SimpleSDAudio.h>

// This version is for board revision 6 in which the two co-processors now communicate with serial.

// Sound

int soundPin = 9;

// MISO MISO CS SCK are used as normal on the SPI port. CS is 10.

int index; // data passed in message block (like {3}), determines track number.

// Our simple comms protocol should allow for 8-16 track selections.
char *AUDIO_FILES[] = {
  "ADMIN.AFM",
  "RELOAD.AFM",
  "SHOOT.AFM", // modify to make sound shorter
  "ALLOUT.AFM", 
  "HIT.AFM",
  "GRENADE.AFM",
  "POWERUP.AFM",
  "POWEROFF.AFM"
};

// State machine for interpreting commands from serial.

Boolean active_packet = 0;

void setup() {

  //pinMode(soundPin, OUTPUT); digitalWrite(soundPin, LOW);

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
  delay(1000);
}


void loop(void) {

  if(Serial.available()){  

    // Read from serial stream. Only expect to use { } and integer numbers.

    char temp = Serial.read();

    if (temp=='{'){active_packet=1;} // start the packet

    else if (temp=='}'){ // finish the packet and execute it

      if (active_packet!=0) { // a second spurious bracket hasn't occured

        active_packet = 0; // close the packet
        if ((index >= 0) && (index < 8) { // sanity check for current sound file options
          play_index(index); // execute
        }
        else {
          Serial.println(F("index outside of acceptable values"));
        }
        index = 0; // Reset index to zero;
      }

    }

    else { // It must be a number or a command character (for later iterations) or garbage serial debug output

        if (active_packet){ // We must be inside a packet

          int temp_int = atoi(temp);
          index = (index*10)+temp_int; // Deals with multi-number requests. Assume only integers for now

        }

    }
  }

}

Boolean play_index(int i){
  if(!SdPlay.setFile(AUDIO_FILES[i])) {
      Serial.println(F(" not found on card! Error code: "));
      Serial.println(SdPlay.getLastError());
    } 
  else {
    Serial.println(F("Command received, playing...")); 
    SdPlay.play();
    while(!SdPlay.isStopped()) {
      ; // no worker needed anymore :-)
    }
  }    
  Serial.println(F("done."));
}
