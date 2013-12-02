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
    PrintDetailsOfSony(results.value);
    Serial.println(results.value, HEX);
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
