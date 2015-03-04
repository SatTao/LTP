// Software for prototype 2, full size

#include <IRremote.h>

// Pin definitions

// Pin 0 and 1 free for RxTx if reqd.

// IR must be on pin 3 to work with the IR library, and it uses interrupt timers.
 
int triggerPin = 2; // Reload and trigger pins must be pulled high as input, and connected through their switches to ground.

int IRPin = 3; // Connected directly to the output IR diode

// LED Bargraph setup (common anode) 

int Serial595Pin = 4;
int latchPin = 5;
int clockPin = 6;

int reloadPin = 7; // Reload and trigger pins must be pulled high as input, and connected through their switches to ground.

int recvPin = 8; // Non-pwm pin for IR recv

int vibePin = 9; // Output pin for vibration (HOLD LOW!)

int reloadLedPin = 10;

// Pins 11, 12, 13 and Reset should be free for programming.

int armedPin = A0;
int muzzlePin = A5;

// Comms with A1 - A4 with co-processor.

int add1 = A4;
int add2 = A3;
int add3 = A2;
int playPin = A1;

// Track codes for music

byte ADMIN = 0;
byte RELOAD = 1;
byte SHOOT = 2;
byte ALLOUT = 3;
byte HIT = 4;
byte GRENADE = 5;
byte POWERON = 6;
byte POWEROFF = 7;

// Activity codes for LED bargraph animation

// Character codes for common anode 7 segment, translated into R2 common anode controller connections.

byte ZERO = B00110000;
byte ONE = B11110101;
byte TWO = B01011000;
byte THREE = B11010000;
byte FOUR = B10010101;
byte FIVE = B10010010;
byte SIX = B00010010;
byte SEVEN = B11110100;
byte EIGHT = B00010000;
byte NINE = B10010000;

byte DP = B11101111;
byte OFF = B11111111;
byte ON = B00000000;

byte HORIZ = B11011010;
byte VERT = B00110101;
byte DASH = B11011111;

// Activity codes for LED 7segment animation

byte NUMBERS[] = {ZERO, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE};

byte CHAMBER_RELOAD[] = {OFF, DASH, VERT, HORIZ, EIGHT, OFF};
byte MAG_RELOAD[] = {OFF, DASH, VERT, HORIZ, EIGHT, OFF}; // TODO Make this better
byte MAG_OUT[] = {ZERO};
byte ALL_OUT[] = {DASH};
byte BEENHIT[] = {OFF, VERT, HORIZ, VERT, HORIZ, VERT, HORIZ, VERT, HORIZ, OFF};

// Global variables

unsigned long reloadFlash;
int reloadFlashDelay = 500;

int roundsPerMag = 9; // ATTENTION! Cannot be greater than 9 until reload logic can handle 2 digits on display
int roundsRemaining = 9;

int magsRemaining = 6;

int team = 1;

unsigned long myFireCode;

// Admin hits state machine

volatile boolean armed = false;
int adminState = 0; // 0 unassigned, 1 Mags, 2 RPM, 3 Team. Acceptable values 0 - 9

// Trigger logic
volatile boolean botherTrigger = false;

// Reload logic
volatile boolean botherReload = false;

IRsend irsend; // IRSend library implementation

IRrecv irrecv(recvPin); // IRrecv library implementation

decode_results results; // results structure for IR recv

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
  pinMode(muzzlePin, OUTPUT); digitalWrite(muzzlePin, LOW);
  
  pinMode(latchPin, OUTPUT); // Let these float for a while, not v important
  pinMode(clockPin, OUTPUT);
  pinMode(Serial595Pin, OUTPUT);
  
  pinMode(reloadLedPin, OUTPUT);

  digitalWrite(armedPin, LOW);
  reloadChamber(); // sets botherTrigger true

  reloadFlash = millis();

  irrecv.enableIRIn(); // Start the receiver

  pinMode(add1, OUTPUT); digitalWrite(add1, HIGH); // Give it plenty of time before doing this.
  pinMode(add2, OUTPUT); digitalWrite(add2, HIGH);
  pinMode(add3, OUTPUT); digitalWrite(add3, HIGH);
  pinMode(playPin, OUTPUT); digitalWrite(playPin, HIGH); // High is inactive

  Serial.println("SETUP COMPLETE!");
}

void loop() {
  if (irrecv.decode(&results)) {
    byte c = 0; byte d = 0;
    boolean ok = extractNEC(&d, &c, results.value);
    Serial.println(results.value, HEX);
    Serial.print("OK?:\t\t"); Serial.println(ok);
    Serial.print("Device code:\t"); Serial.println(d);
    Serial.print("Command code:\t"); Serial.println(c);

    if ((d==0) && ok){
      request_track(0);
      adminHit(c);
    }
    if ( (d>0) && (d<10) && (d!=team) && ok){
      gameHit(d, c); // Friendly Fire is off by default
    }
    if (!ok){
      grazeHit(); // You have been grazed by a shot
    }

    irrecv.resume(); // Receive the next value
  }

  if(Serial.available()){
    char sTemp = Serial.read();
    switch (sTemp){
      case 'f': if (botherTrigger && armed){botherTrigger = false; fire();}; break;
      case 'r': if (botherReload && armed){botherReload = false; reloadMag();}; break;
      default: break;
    }
  }

  if (armed){ // Only check buttons if armed

    if (botherTrigger){ 
      if (checkTrigger()){
        botherTrigger = false;
        fire();
      }
    }
    if (botherReload){
      if (millis() - reloadFlash > reloadFlashDelay) { // Flash the reload button
      digitalWrite(reloadLedPin, !digitalRead(reloadLedPin));
      reloadFlash = millis();
      } 
      if (checkReload()){
        botherReload = false;
        reloadMag();
      }
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
    request_track(2);
    
    // Make a vibration and flash
    digitalWrite(vibePin, HIGH);
    digitalWrite(muzzlePin, HIGH);
    
    // Fire the weapon
    irsend.sendNEC(myFireCode, 32); // NEC code, TODO: make unique by using team designation
    delay(40);
    
    // Cancel the vibration and flash
    digitalWrite(muzzlePin, LOW);
    digitalWrite(vibePin, LOW);

    //Restart the IR detector
    Serial.println("Resuming IR recv");
    irrecv.enableIRIn();
    
    // Clear the status bar
    setLED(OFF);    
    delay(100);
    
    // Check rounds remaining and do the reload logic    
    if (roundsRemaining){reloadChamber();}
    else {request_track(3); animate(MAG_OUT,1,10); delay(1000); botherReload=true;}
  }
}

void reloadChamber(){ // Only gets called if there's definitely a remaining round
  // Show the chamber reload animation in the status 
  Serial.println("RELOAD CHAMBER ROUTINE");
  animate(CHAMBER_RELOAD,6,70);
  if(roundsRemaining < 10) {setLED(NUMBERS[roundsRemaining]);}
  botherTrigger=true;
}

void reloadMag(){
  Serial.println("RELOAD MAG ROUTINE");

  digitalWrite(reloadLedPin, LOW);
  
  // Check mags
  if (magsRemaining) {
    
    // Handle the reload logic
    magsRemaining--;

    // Music
    request_track(1);
    
    // Make a vibration
    digitalWrite(vibePin, HIGH);
    
    // Show the mag reload animation
    animate(MAG_RELOAD,6,70);
    
    // Cancel the vibration
    digitalWrite(vibePin, LOW);

    if(magsRemaining < 10) {setLED(NUMBERS[magsRemaining]);}
    delay(200);
    
    // Handle the reload logic
    roundsRemaining=roundsPerMag;
    reloadChamber();
  }
  else {
    // Give a signal there's no juice left
    request_track(3);
    animate(ALL_OUT,1,10);
    digitalWrite(armedPin, LOW);
    delay(1000);
    // DO NOT set any bother variables true here, result is that main loop continues indefinitly.
  }
}

void setLED(byte output){ // This writes the output verbatim to the 595 - you need to worry about inversion and segment mapping.
  digitalWrite(latchPin, LOW);
  delay(5);
  shiftOut(Serial595Pin, clockPin, LSBFIRST, output);   
  digitalWrite(latchPin, HIGH);
}

void animate(byte *ACTIVITY_CODE, int animation_length, int animation_delay){ // Set LED bargraph to animate the event coded by ACTIVITY_CODE
  // Serial.println("Animating LED output");
  for (int i=0;i<animation_length;i++){
    setLED(ACTIVITY_CODE[i]);
    delay(animation_delay);
  }
}

void adminHit(int c){
  Serial.println("Admin Hit.");
    switch (c) {
        case 13: toggleArmed(); break; // POWER button, toggle armed.
        case 88: setState(1); break; // set mags
        case 86: setState(2); break; // set rounds
        case 87: setState(3); break; // set team
        default: setValue(c); break; // It might be a number or junk, test it.
      }
      
  }

void gameHit(int device, int command){

  request_track(4);
  digitalWrite(vibePin, HIGH);
  animate(BEENHIT,10,200); // Gives roughly 2 second delay.
  digitalWrite(vibePin, LOW);

  // TODO Add reaction to grenade blasts

}

void grazeHit(){} // TODO: What happens if you're grazed

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
  
  digitalWrite(playPin, HIGH); // Reset everything
  digitalWrite(add1, HIGH);
  digitalWrite(add2, HIGH);
  digitalWrite(add3, HIGH);
}

boolean checkTrigger(){
  if (!digitalRead(triggerPin)){ // Trigger is pulled to ground if the switch is closed
    return true;
  }
  else {return false;}
}

boolean checkReload(){
  if (!digitalRead(reloadPin)){ // Reload is pulled to ground if the switch is closed
    return true;
  }
  else {return false;}
}

unsigned long buildNECCommand(byte device, byte command){

  // Device and command are 8 bits -> 0 to 256 are valid values

  if ((device < 0) | (command < 0) | (device > 256) | (command > 256)){
    return 0;
  }

  // Output string format (after the header) is device/~device/command/~command

  unsigned long output = 0;
  byte notdevice = ~device;
  byte notcommand = ~command;

  output |= reverse(device);
  output <<= 8;
  output |= reverse(notdevice);
  output <<= 8;
  output |= reverse(command);
  output <<= 8;
  output |= reverse(notcommand);

  Serial.print("Device was\t"); Serial.println(device, BIN); 
  Serial.print("Command was\t"); Serial.println(command, BIN);
  Serial.print("Output string is:\t"); Serial.println(output, BIN);
  
  return output;

}

boolean extractNEC(byte *device, byte *command, unsigned long code){

  byte dev = code >> 24;
  byte ndev = (code << 8) >> 24;
  byte com = (code << 16) >> 24;
  byte ncom = (code << 24) >> 24;
  
  if (((com & ncom) == 0) && ((dev & ndev) == 0)){ // Everything is as it should be
    *device = reverse(dev); *command = reverse(com);
    return true;
  }
  else if (((dev & ndev) == 0) && ((com & ncom) != 0)){ // Mangled command code from recognised device
    *device = reverse(dev); *command = 0; 
    return false;
  }
  else if (((dev & ndev) != 0) && ((com & ncom) == 0)){ // Mangled device code for a recognised command
    *command = reverse(com); *device = 0; 
    return false;
  }
  else {
    *command = 0; *device = 0;
    return false;
  }
}

byte reverse(byte input){
	byte rev = 0;
  	for(int i=0;i<8;i++){
    	rev <<= 1;
    	rev += input & 1;
    	input >>= 1;
  }
  return rev;
}

void toggleArmed(){
  if(armed){
    delay(1000);
    armed=false; digitalWrite(armedPin, LOW);
    request_track(7);
  }
  else{
    delay(1000);
    armed=true; digitalWrite(armedPin, HIGH);
    request_track(6);
  }
  delay(1000); // Ensure there's time for the music
}

void setState(int s){
  adminState = s;
}

void setValue(int v){
  if ((v >= 0) && (v < 10)) { // Then it is valid
    switch(adminState){
      case 1: magsRemaining = v; roundsRemaining = roundsPerMag; break;
      case 2: roundsPerMag = v; roundsRemaining = roundsPerMag; break;
      case 3: team = v; break;
      default: break;
    }
  }
}

// End
