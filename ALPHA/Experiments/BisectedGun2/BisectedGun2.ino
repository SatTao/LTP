// Software to run on bisected prototype gun BLH 1, from breadboarded arduino UNO. No longer requires interrupts or multiplexor.

#include <IRremote.h>

// Pin definitions

int armedPin = A3;

// Trigger, reload and IR pins. IR must be on pin 3 to work with the IR library, and it uses interrupt timers.
 
int triggerPin = 2; // Does not use the interrupt
int IRPin = 3; // Connected directly to the output IR diode
int reloadPin = 7; // Reload and trigger pins must be pulled high as input, and connected through their switches to ground.

int reloadLedPin = 10;
// Output pin for vibration (HOLD LOW!)

int vibePin = 9;

// Output pin for sound, activates cheap sound card, hold low!

int soundPin = 8;

// LED Bargraph setup (common cathode)

int Serial595Pin = 4;
int latchPin = 5;
int clockPin = 6;

// Activity codes for LED bargraph animation

//byte CHAMBER_RELOAD[] = {0, B00000001, B00000011, B00000111, B00001111, B00011111, B00111111, B01111111, B11111111};
//byte MAG_RELOAD[] = {0, B10000001, B11000011, B11100111, B11111111, B01111110, B00111100, B00011000, 0, 1, 0};
//byte MAG_OUT[] = {B00000011};
//byte ALL_OUT[] = {B10000000};

// Character codes for common anode 7 segment, translated into vertical controller connections.



byte ZERO = B01000010;
byte ONE = B11111010;
byte TWO = B00110111;
byte THREE = B00110010;
byte FOUR = B10011010;
byte FIVE = B00010110;
byte SIX = B00000110;
byte SEVEN = B11110010;
byte EIGHT = B00000010;
byte NINE = B00010010;

byte DP = B11111101;
byte OFF = B11111111;

byte HORIZ = B00110111;
byte VERT = B11001010;
byte DASH = B10111111;

// Activity codes for LED 7segment animation

byte NUMBERS[] = {ZERO, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE};

byte CHAMBER_RELOAD[] = {OFF, DASH, VERT, HORIZ, EIGHT, OFF};
byte MAG_RELOAD[] = {OFF, DASH, VERT, HORIZ, EIGHT, OFF}; // TODO Make this better
byte MAG_OUT[] = {ZERO};
byte ALL_OUT[] = {DASH};

// Global variables

int roundsPerMag = 9; // ATTENTION! Cannot be greater than 9 until reload logic can handle 2 digits on display
int roundsRemaining = 9;

int magsRemaining = 6;

unsigned long myFireCode;

// Trigger logic
volatile boolean botherTrigger = false;

// Reload logic
volatile boolean botherReload = false;

IRsend irsend; // IRSend library implementation

void setup() {
  delay(1000);
  pinMode(armedPin, OUTPUT);
  digitalWrite(armedPin, HIGH); // Indicates device active
  Serial.begin(9600);
  delay(500);
  Serial.println("SETUP");
  Serial.println("Building local NEC fire code, incorporating device 8 command 234");
  myFireCode = buildNECCommand(8,234);
  Serial.println(myFireCode, HEX);
  
  pinMode(IRPin, OUTPUT); digitalWrite(IRPin, LOW);
  
  pinMode(triggerPin, INPUT); digitalWrite(triggerPin, HIGH); // set as input with active pullup
  pinMode(reloadPin, INPUT); digitalWrite(reloadPin, HIGH);
  
  pinMode(vibePin, OUTPUT); digitalWrite(vibePin, LOW);
  pinMode(soundPin, OUTPUT); digitalWrite(soundPin, LOW);
  
  pinMode(latchPin, OUTPUT); // Let these float for a while, not v important
  pinMode(clockPin, OUTPUT);
  pinMode(Serial595Pin, OUTPUT);

  digitalWrite(armedPin, LOW);
  reloadChamber(); // sets botherTrigger true
  digitalWrite(armedPin, HIGH);

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
    
    // Make a noise
    digitalWrite(soundPin, HIGH);
    
    // Make a vibration
    digitalWrite(vibePin, HIGH);
    
    // Fire the weapon
    irsend.sendNEC(myFireCode, 32); // NEC code, TODO: make unique by using values held in eeprom
    delay(40);
    
    // Cancel the vibration
    digitalWrite(vibePin, LOW);
    
    // Clear the status bar
    setLED(OFF);    
    delay(100);
    
    // Cancel the noise
    
    digitalWrite(soundPin, LOW);
    
    // Check rounds remaining and do the reload logic    
    if (roundsRemaining){reloadChamber();}
    else {animate(MAG_OUT,1,10); botherReload=true;}
  }
}

void reloadChamber(){ // Only gets called if there's definitely a remaining round
  // Show the chamber reload animation in the status 
  Serial.println("RELOAD CHAMBER ROUTINE");
  animate(CHAMBER_RELOAD,6,70);
  if(roundsRemaining < 10) {setLED{NUMBERS[roundsRemaining]};}
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
    animate(MAG_RELOAD,6,70);
    
    // Cancel the vibration
    digitalWrite(vibePin, LOW);

    if(magsRemaining < 10) {setLED{NUMBERS[magsRemaining]};}
    delay(200);
    
    // Handle the reload logic
    roundsRemaining=roundsPerMag;
    reloadChamber();
  }
  else {
    // Give a signal there's no juice left
    animate(ALL_OUT,1,10);
    digitalWrite(armedPin, LOW);
    // DO NOT set any bother variables true here, result is that main loop continues indefintely.
  }
}

void setLED(byte output){ // This writes the output verbatim to the 595 - you need to worry about inversion and segment mapping.
  digitalWrite(latchPin, LOW);
  delay(5);
  shiftOut(Serial595Pin, clockPin, MSBFIRST, output);   
  digitalWrite(latchPin, HIGH);
}

void set7seg(byte output){} // Placeholder

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

unsigned long buildNECCommand(byte device, byte command){

  // Device and command are 8 bits -> 0 to 256 are valid values

  if (device < 0 | command < 0 | device > 256 | command > 256){
    return 0;
  }

  // Output string format (after the header) is device/~device/command/~command

  unsigned long output = 0;
  byte notdevice = ~device;
  byte notcommand = ~command;

  output |= device;
  output <<= 8;
  output |= notdevice;
  output <<= 8;
  output |= command;
  output <<= 8;
  output |= notcommand;

  Serial.print("Device was\t"); Serial.println(device, BIN); 
  Serial.print("Command was\t"); Serial.println(command, BIN);
  Serial.print("Output string is:\t"); Serial.println(output, BIN);
  
  return output;

}
// End
