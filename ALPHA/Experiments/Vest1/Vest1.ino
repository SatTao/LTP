// Vest 1

// Main control software for prototype vest.

// Vest needs to store some identity information, listen for IR NEC codes, 
// communicate via serial with attached guns, and control lighting/vibration.

// Option for touchscreen control too, or extended PWM control of more lighting.

// Imports

#include <IRremote.h>

// IR defintions

int RECV_PIN = 12;

IRrecv irrecv(RECV_PIN);

decode_results results;

// LED PWM defintions (PWM on 3,5,6,9,10,11)

int left_red = 3;
int left_green = 5;
int left_blue = 6;

int right_red = 9;
int right_green = 10;
int right_blue = 11;

// Vibration defintions

int vibe = 7;

// NOTE pins 0 and 1 are hardcoded as serial comms: we will use them as such.
