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

volatile boolean armed = false;
volatile int state = 0; // 0: Null, 1: setmags, 2: setroundspermag, 3: gunID, 4: teamID
int mags = 0;
int rpm = 0;
int id = 0;
int team = 0;

long int last_val_time = 0; // Controls decimal addition in state machine, time of 3 seconds.

// State machine for colour control. Allows for planning a follow-up program

int program = 0; // 0: Null, 1: constant, 2: crossfade, 3: pulse
int program_ahead = 0; // Same as above
int repeats = -1; // -1: unlimited 0: ready for next program, 1-N: number
int repeats_ahead; // behaviour is to go Null program after _ahead is completed, if this is non-zero.
byte *const_col = BLACK;
byte *const_col_ahead = BLACK;
byte *cross_from = BLACK;
byte *cross_from_ahead = BLACK;
byte *cross_to = BLACK;
byte *cross_to_ahead = BLACK;
byte *pulse_from = BLACK;
byte *pulse_from_ahead = BLACK;
byte *pulse_to = BLACK;
byte *pulse_to_ahead = BLACK;
int steps = 1; // How many steps to perform crossfades/pulses in.
int step_count = 0; // How far are we in the program already.
int steps_ahead = 1;
int millis_per_step = 10; // How many milliseconds per step in crossfades/pulses
int millis_per_step_ahead = 10; // So intended program time is repeats*steps*millis_per_step
int pulse_stage = 0; // For use by pulse program. Indicates whether we're rising (0), apex (1) or fading (2)

long int systime; // Keep track of lighting steps by looking at system time.

// NOTE pins 0 and 1 are hardcoded as serial comms: we will use them as such, eventually.

void setup()
{
  Serial.begin(9600);

  pinMode(left_red, OUTPUT);
  pinMode(left_green, OUTPUT);
  pinMode(left_blue, OUTPUT);

  pinMode(RECV_PIN, INPUT);

  delay(1000);

  systime = millis();
  irrecv.enableIRIn(); // Start the receiver
  bump_disarmed();
  Serial.println("STARTUP");
}

void loop() {

	if (program && (millis() - systime >= millis_per_step)) { // If there is a lighting program running at the moment, and the timer indicates it.

		if (repeats == 0){ // Start the next program.
			bump_ahead();
                        switch(program){
				case 1: copy_from(const_col, CC); break; //constant
				case 2: break; //crossfade
				case 3: copy_from(pulse_from, CC); break; //pulse
				default: break; // Do nothing if the program is not recognised.
			}
		}
		else {
			switch(program){ // TODO
				case 1: p_constant(); break; //constant
				case 2: break; //crossfade
				case 3: p_pulse(); break; //pulse
				default: break; // Do nothing if the program is not recognised.
			}
			systime = millis();
		}
	}

	// TODO: Store hits info in sensible format.


	if (irrecv.decode(&results)) {
		byte c = 0; byte d = 0;
                
		if (extractNEC(&d, &c, results.value)) {
			Serial.println(results.value, HEX);
			if (d==0) { // Then this is a system message
				
				Serial.println("ADMIN HIT");
				//bump_admin_hit();

				switch (c) {
					case 20: printState(); break; // MENU button, print out the state of the device.
					case 13: toggleArmed(); break; // POWER button, toggle armed.
					case 88: setState(1); break; // set mags
					case 86: setState(2); break; // set rounds
					case 87: setState(3); break; // set id
					case 89: setState(4); break; // set team
					case 14: break; // TODO: Add data reset function
					default: setValue(c); break; // It might be a number or junk, test it.
				}
			}
			else { // This is a hit from another gun
				Serial.print("GUN HIT:\t");
				Serial.print(d); Serial.print("\t"); Serial.println(c);
				//bump_armed_hit();
				// TODO: game logic and game hit indication of state
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
	if (armed) {bump_disarmed(); Serial.println("Disarmed");}
	else {bump_armed(); Serial.println("Armed");}

	armed = !armed;
}

void setState(int s){
	state = s;
}

void setValue(int v){
	if ((v >= 0) && (v < 10)) { // Then it is valid
		if (millis() - last_val_time < 3000){ // Recent button press, decimal system adding, only two powers
			switch(state){
				case 1: mags = (mags*10) + v; state = 0; break;
				case 2: rpm = (rpm*10) + v; state = 0; break;
				case 3: id = (id*10) + v; state = 0; break;
				case 4: team = (team*10) + v; state = 0; break;
				default: break;
			}
		}
		else {
			switch(state){
				case 1: mags = v; break;
				case 2: rpm = v; break;
				case 3: id = v; break;
				case 4: team = v; break;
				default: break;
			}

			last_val_time = millis();
		}
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

void setcol(byte *col){
  analogWrite(left_red, col[0]);
  analogWrite(left_green, col[1]);
  analogWrite(left_blue, col[2]);
}

void bump_ahead(){
	
	// Moves data from _ahead to current in the lighting program.
        Serial.println("bump_ahead");

	program = program_ahead;
	program_ahead = 0; 
	repeats = repeats_ahead;
	repeats_ahead = -1;
	const_col = const_col_ahead;
	const_col_ahead = BLACK;
	cross_from = cross_from_ahead;
	cross_from_ahead = BLACK;
	cross_to = cross_to_ahead;
	cross_to_ahead = BLACK;
	pulse_from = pulse_from_ahead;
	pulse_from_ahead = BLACK;
	pulse_to = pulse_to_ahead;
	pulse_to_ahead = BLACK;
	steps = steps_ahead;
	step_count = 0;
	steps_ahead = 1;
	millis_per_step = millis_per_step_ahead;
	millis_per_step_ahead = 10;
	pulse_stage = 0;
}

void team_ahead(){

	// Sets up team as next lighting program

        Serial.println("team_ahead");

	// Pulsing red colour over 2 seconds

	program_ahead = 3;
	repeats_ahead = -1;
	pulse_from_ahead = BLACK;
	pulse_to_ahead = RED; // TODO Make this dynamic
	steps_ahead = 200;
	millis_per_step_ahead = 10;
}

void disarmed_ahead(){

	// Sets up disarmed as next lighting program

        Serial.println("disarmed_ahead");

	// Constant white colour

	program_ahead = 1;
	repeats_ahead = -1;
	const_col_ahead = WHITE;
	steps_ahead = 1;
	millis_per_step_ahead = 1000;
}

void bump_armed(){

	// Bump an armed indication and line up team colours ahead

        Serial.println("bump_armed");

	// Armed indication is 3 short white to red pulses

	program = 3;
	repeats = 3;
	pulse_from = WHITE;
	pulse_to = RED;
	steps = 100;
	step_count = 0;
	millis_per_step = 5;
	pulse_stage = 0;

	team_ahead();
}

void bump_disarmed(){

	// Bump a disarmed indication and line up a disarmed indication

        Serial.println("bump_disarmed");

	// Disarmed indication is 3 short red to green pulses

	program = 3;
	repeats = 3;
	pulse_from = RED;
	pulse_to = GREEN;
	steps = 100;
	step_count = 0;
	millis_per_step = 5;
	pulse_stage = 0;

	disarmed_ahead();
}

void bump_admin_hit(){

	// Bump an admin hit indication and line up the existing state

        Serial.println("bump_admin_hit");

	// Admin indication is 1 medium white to blue pulse

	program = 3;
	repeats = 1;
	pulse_from = WHITE;
	pulse_to = BLUE;
	steps = 100;
	step_count = 0;
	millis_per_step = 8;
	pulse_stage = 0;

	if (armed) {team_ahead();}
	else {disarmed_ahead();}
}

void bump_armed_hit(){

	// Bump an armed hit indication and line up the existing state

        Serial.println("bump_armed_hit");

	// Armed indication is 1 medium red to blue pulse

	program = 3;
	repeats = 1;
	pulse_from = RED;
	pulse_to = BLUE;
	steps = 100;
	step_count = 0;
	millis_per_step = 8;
	pulse_stage = 0;

	if (armed) {team_ahead();}
	else {disarmed_ahead();}
}

void p_constant(){

	// Sets outputs for constant lighting, and updates control parameters.

	copy_from(const_col, CC);
	setcol(CC);

	step_count++;
	if (step_count == steps) { // Then this stage is over
		step_count = 0;
		if (repeats > 0){repeats--;}
	}

}

void p_crossfade(){
	// TODO
	// Sets outputs for crossfade lighting, and updates control parameters.

}

void p_pulse(){
	
	// Sets outputs for pulse lighting, and updates control parameters.

	int upsteps= floor(steps/2);

	int delta_red = (byte)floor((pulse_to[0] - pulse_from[0]) / upsteps);
	int delta_green = (byte)floor((pulse_to[1] - pulse_from[1]) / upsteps);
	int delta_blue = (byte)floor((pulse_to[2] - pulse_from[2]) / upsteps);

	byte newc[] = {0,0,0};

	if (pulse_stage==0) { // We're going up
		newc[0] = CC[0] + delta_red;
		newc[1] = CC[1] + delta_green;
		newc[2] = CC[2] + delta_blue;
		copy_from(newc, CC); 
		setcol(CC);
	}
	else if (pulse_stage==1){ // We're at the apex
		copy_from(pulse_to, newc);
		copy_from(newc, CC);
		pulse_stage = 2;
	}
	else { // We're going down
		newc[0] = CC[0] - delta_red;
		newc[1] = CC[1] - delta_green;
		newc[2] = CC[2] - delta_blue;
		copy_from(newc, CC);
		setcol(CC);
	}

	step_count++;
	if (step_count = upsteps){pulse_stage=1;} // Invert
	if (step_count = steps){ // Clean up and reset
		step_count = 0;
		pulse_stage = 0;
		if (repeats > 0){repeats--;}
	}

}

void copy_from(byte *from, byte *to){
	to[0] = from[0];
	to[1] = from[1];
	to[2] = from[2];
}
