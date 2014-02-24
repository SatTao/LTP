// Vest 1

// Main control software for prototype vest.

// Vest needs to store some identity information, listen for IR NEC codes, 
// communicate via serial with attached guns, and control lighting/vibration.

// Option for touchscreen control too, or extended PWM control of more lighting.

// Imports

#include <IRremote.h>

// IR defintions

int RECV_PIN = 12; // Because we don't need a PWM pin for this.

IRrecv irrecv(RECV_PIN);

decode_results results;

// LED PWM defintions (PWM on 3,5,6,9,10,11)

int left_red = 3; // Only use a single strip for testing.
int left_green = 5;
int left_blue = 6;

//int right_red = 9;
//int right_green = 10;
//int right_blue = 11;

byte RED[] = {255,0,0};
byte GREEN[] = {0,255,0};
byte BLUE[] = {0,0,255};
byte PURPLE[] = {255,0,255};
byte TURQUOISE[] = {0,255,255};
byte YELLOW[] = {255,255,0};
byte WHITE[] = {255,255,255};
byte BLACK[] = {0,0,0};

byte CC[] = {0,0,0}; // Current colour configuration (8bitRGB)

// Vibration defintions

int vibe = 7;

// State machine for input data

boolean armed = false;
int state = 0; // 0: Null, 1: setmags, 2: setroundspermag, 3: gunID, 4: teamID
int mags = 0;
int rpm = 0;
int id = 0;
int team = 0;

// State machine for colour control. Allows for planning a follow-up program

int program = 0; // 0: Null, 1: constant, 2: crossfade, 3: pulse
int program_ahead = 0; // Same as above
int repeats = 0; // 0: unlimited, 1-N: number
int repeats_ahead; // behaviour is to go Null program after _ahead is completed, if this is non-zero.
byte const_col[] = BLACK;
byte const_col_ahead[] = BLACK;
byte cross_from[] = BLACK;
byte cross_from_ahead[] = BLACK;
byte cross_to[] = BLACK;
byte cross_to_ahead[] = BLACK;
byte pulse_from[] = BLACK;
byte pulse_from_ahead[] = BLACK;
byte pulse_to[] = BLACK;
byte pulse_to_ahead[] = BLACK;
int steps = 1; // How many steps to perform crossfades/pulses in
int steps_ahead = 1;
int millis_per_step = 10; // How many milliseconds per step in crossfades/pulses
int millis_per_step_ahead = 10; // So intended program time is repeats*steps*millis_per_step
int pulse_stage = 0; // For use by pulse program. Indicates whether we're rising (0) or fading (1)

// NOTE pins 0 and 1 are hardcoded as serial comms: we will use them as such, eventually.

void setup()
{
  Serial.begin(9600);

  pinMode(left_red, OUTPUT);
  pinMode(left_green, OUTPUT);
  pinMode(left_blue, OUTPUT);

  delay(1000);

  irrecv.enableIRIn(); // Start the receiver
}

void loop() {


	// TODO: Include lighting control. Team colours, hit indications, armed indication, admin data received, etc

	// TODO: Store hits info in sensible format.


	if (irrecv.decode(&results)) {
		byte c = 0; byte d = 0;
                Serial.println("HIT");
		if (extractNEC(&d, &c, results.value)) {
			Serial.println(results.value, HEX);
			if (d==0) { // Then this is a system message
				switch (c) {
					case 20: printState(); break; // MENU button, print out the state of the device.
					case 13: toggleArmed(); break; // POWER button, toggle armed.
					case 88: setState(1); break; // set mags
					case 86: setState(2); break; // set rounds
					case 87: setState(3); break; // set id
					case 89: setState(4); break; // set team
					default: setValue(c); break; // It might be a number or junk, test it.
					// TODO: Add reset code to remove hits data.
				}
			}
			else { // This is a hit from another gun
				// TODO: game logic
			}
		}
		irrecv.resume(); // Receive the next value
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

void printState(){
	Serial.print("Armed? :\t"); Serial.println(armed);
	Serial.print("Team :\t"); Serial.println(team);
	Serial.print("ID :\t"); Serial.println(id);
	Serial.print("Mags :\t"); Serial.println(mags);
	Serial.print("Rounds per mag :\t"); Serial.println(rpm);
	// TODO: Add print data section, displaying number of received hits etc.
}

void toggleArmed(){
	armed=!armed;
	// TODO: Add lighting indication of state
}

void setState(int s){
	state = s;
}

void setValue(int v){
	if ((v >= 0) && (v < 10)) { // Then it is valid
		switch(state){
			case 1: mags = v; state = 0; break;
			case 2: rpm = v; state = 0; break;
			case 3: id = v; state = 0; break;
			case 4: team = v; state = 0; break;
			default: break;
		}
	}
}

// TODO: allow for multi-digit entries in setValue. i.e. x = 10*x + y

byte reverse(byte input){
	byte rev = 0;
  	for(int i=0;i<8;i++){
    	rev <<= 1;
    	rev += input & 1;
    	input >>= 1;
  }
  return rev;
}

void setcol(byte *col){
  analogWrite(left_red, col[0]);
  analogWrite(left_green, col[1]);
  analogWrite(left_blue, col[2]);
}

