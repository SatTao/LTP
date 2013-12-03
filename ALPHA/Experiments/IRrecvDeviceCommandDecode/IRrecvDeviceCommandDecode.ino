/*
 * IRremote: IRrecvDemo - demonstrates receiving IR codes with IRrecv
 * An IR detector/demodulator must be connected to the input RECV_PIN.
 * Version 0.1 July, 2009
 * Copyright 2009 Ken Shirriff
 * http://arcfn.com
 */

#include <IRremote.h>

int RECV_PIN = 11;

IRrecv irrecv(RECV_PIN);

decode_results results;

void setup()
{
  Serial.begin(9600);
  irrecv.enableIRIn(); // Start the receiver
}

void loop() {
  if (irrecv.decode(&results)) {
    c = 0; d = 0;
    boolean ok = extractNEC(&d, &c, results.value);
    Serial.println(results.value, HEX);
    Serial.print("OK?:\t\t"); Serial.println(ok);
    Serial.print("Device code:\t"); Serial.println(d);
    Serial.print("Command code:\t"); Serial.println(c);
    irrecv.resume(); // Receive the next value
  }
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

boolean extractNEC(byte *device, byte *command, unsigned long code){

  byte dev = code >> 24;
  byte ndev = (code << 8) >> 24;
  byte com = (code << 16) >> 24;
  byte ncom = (code << 24) >> 24;
  
  if (((com & ncom) == 0) && ((dev & ndev) == 0)){ // Everything is as it should be
    *device = dev; *command = com;
    return true;
  }
  else if (((dev & ndev) == 0) && ((com & ncom) != 0)){ // Mangled command code from recognised device
    *device = dev; *command = 0; 
    return false;
  }
  else if (((dev & ndev) != 0) && ((com & ncom) == 0)){ // Mangled device code for a recognised command
    *command = com; *device = 0; 
    return false;
  }
  else {
    *command = 0; *device = 0;
    return false;
  }
}