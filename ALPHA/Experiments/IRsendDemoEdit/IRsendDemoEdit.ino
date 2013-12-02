/*
 * IRremote: IRsendDemo - demonstrates sending IR codes with IRsend
 * An IR LED must be connected to Arduino PWM pin 3.
 * Version 0.1 July, 2009
 * Copyright 2009 Ken Shirriff
 * http://arcfn.com
 */

#include <IRremote.h>

IRsend irsend;

void setup()
{
}

void loop() {
  if (1) {
    for (int i = 0; i < 3; i++) {
      irsend.sendSony(0x19E, 12); // Sony TV power code
      delay(5000);
    }
  }
}
