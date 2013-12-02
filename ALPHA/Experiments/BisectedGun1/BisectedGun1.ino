// Software to run on bisected prototype gun BLH 1, from breadboarded arduino UNO.

#include <IRremote.h>

// Pin definitions

// Interrupt pins for trigger and reload.

int triggerPin = 3; // NOTE This pin is also the IR output. See multiplex circuit.
int reloadPin = 2; // May need external pullup on shop UNO, smoked internal pullup (?)

// IR control circuit pin (includes triggerPin)

int IREnablePin = 7;
int IRHelperPin = 8;

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
volatile int triggerFlag = LOW;
volatile int botherTrigger = LOW;

// Reload logic
volatile int reloadFlag = LOW;
volatile int botherReload = LOW;

IRsend irsend; // IRSend library implementation

void setup() {
  Serial.begin(9600);
  delay(500);
  Serial.println("SETUP");
  
  pinMode(triggerPin, INPUT); digitalWrite(triggerPin, HIGH); // set as input with active pullup
  pinMode(reloadPin, INPUT); digitalWrite(reloadPin, HIGH);
  
  pinMode(vibePin, OUTPUT); digitalWrite(vibePin, LOW);

  pinMode(IREnablePin, OUTPUT); digitalWrite(IREnablePin, LOW);
  pinMode(IRHelperPin, OUTPUT); digitalWrite(IRHelperPin, LOW);
  
  pinMode(latchPin, OUTPUT); // Let these float for a while, not v important
  pinMode(clockPin, OUTPUT);
  pinMode(Serial595Pin, OUTPUT);

  reloadChamber();

  setTriggerActive(); // Set multiplex circuit to trigger mode
  botherTrigger=HIGH;
  
  attachInterrupt(1,ISR_trigger,FALLING); // attach interrupt service routines for trigger and reload logic
  attachInterrupt(0,ISR_reload,FALLING); // Interrupt 0 is pin 2, 1 is pin 3.

  Serial.println("SETUP COMPLETE!");
}

void loop() {
  if (triggerFlag){ // Only raised by ISR_trigger, which sets bother low too
    triggerFlag=LOW; 
    // cli(); // Clear all interrupts, so we're not distracted by any trigger/reload pulls.
    fire();    
  }
  if (reloadFlag){ // Only raised by ISR_reload, which sets bother low too
    reloadFlag=LOW; 
    reloadMag();
  }
} 

// Interrupt service routines for trigger and reload
void ISR_trigger(){
  if (botherTrigger) {cli(); botherTrigger=LOW;triggerFlag=HIGH;Serial.println("ISR:\tTRIGGER");}
}

void ISR_reload(){
  if (botherReload) {botherReload=LOW;reloadFlag=HIGH;Serial.println("ISR:\tRELOAD");}
}

void fire(){ // By this point all interrupts should be disabled so we can safely use the trigger pin for IR transmitting
  // Check rounds
  Serial.println("FIRE ROUTINE");
  if (roundsRemaining){ // Bother reload will have been set earlier if we're out of rounds now
    
    // Handle the trigger logic
    roundsRemaining--;
    
    // Make a vibration
    digitalWrite(vibePin, HIGH);

    // Reset multiplex circuit to IR transmit mode
    setIRActive();
    delay(10);
    detachInterrupt(1);
    detachInterrupt(0);
    delay(10);
    
    // Fire the weapon
    //for (int t=0;t<3;t++) {
      irsend.sendSony(0xa90, 12); // Sony TV power code for now TODO: Change slightly
      delay(40);
    //}

    // Reset multiplex circuit to trigger mode
    attachInterrupt(1,ISR_trigger,FALLING);
    attachInterrupt(0,ISR_reload,FALLING);
    setTriggerActive();
    
    // Make a noise
    
    // Cancel the vibration
    digitalWrite(vibePin, LOW);
    
    // Clear the status bar
    setLED(0);    
    delay(100);
    
    // Check rounds remaining and do the reload logic    
    if (roundsRemaining){reloadChamber();}
    else {animate(MAG_OUT,1,10); botherReload=HIGH;}
  }
}

void reloadChamber(){ // Only gets called if there's definitely a remaining round
  // Show the chamber reload animation in the status 
  Serial.println("RELOAD CHAMBER ROUTINE");
  animate(CHAMBER_RELOAD,9,70);
  botherTrigger=HIGH;
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
    while(1){;} // HANG
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

void setTriggerActive(){ // Prepare multiplex circuit for listening to the trigger
  Serial.println("Setting Trigger Active");
  digitalWrite(IREnablePin, LOW); // Turn off IR signals
  pinMode(triggerPin, INPUT); digitalWrite(triggerPin, HIGH); // Set pullup 20kOhms to 5v
  digitalWrite(IRHelperPin, LOW); // Creates voltage difference across trigger
  delay(100);
  // Serial.print("Trigger pin reads:\t"); Serial.println(digitalRead(triggerPin));
  sei(); // Sets all interrupts on (enables the trigger and reload ISRs)
} 

void setIRActive(){ // Prepare multiplex circuit for transmitting on the trigger pin
  // Interrupts should have been disabled by now 
  Serial.println("Setting IR Active");
  digitalWrite(IRHelperPin, HIGH);
  digitalWrite(triggerPin, LOW);
  pinMode(triggerPin, OUTPUT);
  digitalWrite(IREnablePin, HIGH);
}

// END
