// Software to run on bisected prototype gun BLH 1, from breadboarded arduino UNO. No longer requires interrupts or multiplexor.

#include <IRremote.h>

// Pin definitions

// Trigger, reload and IR pins. IR must be on pin 3 to work with the IR library, and it uses interrupt timers.
 
int triggerPin = 2; // Does not use the interrupt
int IRPin = 3; // Connected directly to the output IR diode
int reloadPin = 7; // Reload and trigger pins must be pulled high as input, and connected through their switches to ground.

// Output pin for vibration (HOLD LOW!)

int vibePin = 9;

// LED Bargraph setup (common cathode)

int Serial595Pin = 4;
int latchPin = 5;
int clockPin = 6;

// Activity codes for LED bargraph animation

byte CHAMBER_RELOAD[] = {0, B00000001, B00000011, B00000111, B00001111, B00011111, B00111111, B01111111, B11111111};
byte MAG_RELOAD[] = {0, B10000001, B11000011, B11100111, B11111111, B01111110, B00111100, B00011000, 0, 1, 0};
byte MAG_OUT[] = {B00000011};
byte ALL_OUT[] = {B10000000};

// Global variables

int roundsPerMag = 12;
int roundsRemaining = 12;

int magsRemaining = 6;

// Trigger logic
volatile boolean botherTrigger = false;

// Reload logic
volatile boolean botherReload = false;

IRsend irsend; // IRSend library implementation

void setup() {
  Serial.begin(9600);
  delay(500);
  Serial.println("SETUP");
  
  pinMode(IRPin, OUTPUT); digitalWrite(IRPin, LOW);
  
  pinMode(triggerPin, INPUT); digitalWrite(triggerPin, HIGH); // set as input with active pullup
  pinMode(reloadPin, INPUT); digitalWrite(reloadPin, HIGH);
  
  pinMode(vibePin, OUTPUT); digitalWrite(vibePin, LOW);
  
  pinMode(latchPin, OUTPUT); // Let these float for a while, not v important
  pinMode(clockPin, OUTPUT);
  pinMode(Serial595Pin, OUTPUT);

  reloadChamber(); // sets botherTrigger true

  Serial.println("SETUP COMPLETE!");
}

void loop() {
  if (botherTrigger){ 
    if (checkTrigger()){
      botherTrigger = false;
      fire();
    }
  }
  if (botherReload){
    if (checkReload()){
      botherReload = false;
      reloadMag();
    }
  }
}

void fire(){
  // Check rounds
  Serial.println("FIRE ROUTINE");
  if (roundsRemaining){ // Bother reload will have been set earlier if we're out of rounds now
    
    // Handle the trigger logic
    roundsRemaining--;
    
    // Make a vibration
    digitalWrite(vibePin, HIGH);
    
    // Fire the weapon
    //for (int t=0;t<3;t++) {
      irsend.sendSony(0xa90, 12); // Sony TV power code for now TODO: Change slightly
      delay(40);
    //}
    
    // Make a noise
    
    // Cancel the vibration
    digitalWrite(vibePin, LOW);
    
    // Clear the status bar
    setLED(0);    
    delay(100);
    
    // Check rounds remaining and do the reload logic    
    if (roundsRemaining){reloadChamber();}
    else {animate(MAG_OUT,1,10); botherReload=true;}
  }
}

void reloadChamber(){ // Only gets called if there's definitely a remaining round
  // Show the chamber reload animation in the status 
  Serial.println("RELOAD CHAMBER ROUTINE");
  animate(CHAMBER_RELOAD,9,70);
  botherTrigger=true;
}

void reloadMag(){
  Serial.println("RELOAD MAG ROUTINE");
  // Check mags
  if (magsRemaining) {
    
    // Handle the reload logic
    magsRemaining--;
    
    // Make a vibration
    digitalWrite(vibePin, HIGH);
    
    // Show the mag reload animation
    animate(MAG_RELOAD,11,70);
    
    // Cancel the vibration
    digitalWrite(vibePin, LOW);
    
    // Handle the reload logic
    roundsRemaining=roundsPerMag;
    reloadChamber();
  }
  else {
    // Give a signal there's no juice left
    animate(ALL_OUT,1,10);
    // DO NOT set any bother variables true here, result is that main loop continues indefintely.
  }
}

void setLED(byte output){
  digitalWrite(latchPin, LOW);
  delay(5);
  shiftOut(Serial595Pin, clockPin, MSBFIRST, output);   
  digitalWrite(latchPin, HIGH);
}

void animate(byte *ACTIVITY_CODE, int animation_length, int animation_delay){ // Set LED bargraph to animate the event coded by ACTIVITY_CODE
  // Serial.println("Animating LED output");
  for (int i=0;i<animation_length;i++){
    setLED(ACTIVITY_CODE[i]);
    delay(animation_delay);
  }
}

boolean checkTrigger(){
  if (!digitalRead(triggerPin)){ // Trigger is pulled to ground if the switch is closed
    return true;
  }
  else {return false;}
}

boolean checkReload(){
  if (!digitalRead(reloadPin)){ // Trigger is pulled to ground if the switch is closed
    return true;
  }
  else {return false;}
}

// This is a test line to determine whether Git notices small changes in local repositories.
// END

int buildNECCommand(byte device, byte command){


  // TODO: test device and command are in range.

  int output = 0;
  byte notdevice = ~device;
  byte notcommand = ~command;

  output |= device;
  output <<= 8;
  output |= notdevice;
  output <<= 8;
  output |= command;
  output <<= 8;
  output |= notcommand;
  output <<= 8;

  // TODO: Check this produces something reasonable for debugging

  // TODO: Build a decoder for the receiver, and test it. Can do partial matches as near misses.
}


}