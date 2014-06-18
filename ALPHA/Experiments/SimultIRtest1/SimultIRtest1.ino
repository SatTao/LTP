// simultaneous send recv IR test

#include <IRremote.h>

int RECV_PIN = 11;

int TRANS_PIN = 3; // This one's hardcoded

int trigger_pin = 8; // Listen for this going to ground.

// This sketch tests the simlutaneous instantiation of IRsend and IRrecv objects, and their 
// subsequent use.

IRrecv irrecv(RECV_PIN); 

decode_results results;

IRsend irsend;

// This sketch is intended to mimic very simply what the gun has to do: listen for incoming shots all the time,
// fire occasionally (sometimes in response to those shots), and maintain some other pins.

// The hardware setup should also test multiple IR sensors on the same, long line.

void setup()
{
  pinMode(trigger_pin, OUTPUT);
  digitalWrite(trigger_pin, HIGH); // Pullup to 5v
  Serial.begin(9600);
  irrecv.enableIRIn(); // Start the receiver
}

void loop() {
  if (irrecv.decode(&results)) {
    byte c = 0; byte d = 0;
    boolean ok = extractNEC(&d, &c, results.value);
    Serial.println(results.value, HEX);
    Serial.print("OK?:\t\t"); Serial.println(ok);
    Serial.print("Device code:\t"); Serial.println(d);
    Serial.print("Command code:\t"); Serial.println(c);

    // Device code of 0 means this is the TV remote hitting it.

    irrecv.resume(); // Receive the next value
  }

  if (!digitalRead(trigger_pin)){
  	// The trigger has been pulled

  	// irrecv should stop automatically

  	unsigned long u = B1010101; // Some random number

  	irsend.sendNEC(u, 32);
  	Serial.println("Sending a signal");
  	delay(40); // Enough to send it, probably.
  	Serial.println("Resuming IR recv");
  	irrecv.enableIRIn(); // To restart the receiver
  }
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
