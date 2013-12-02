
// setup pins and variables for SRF05 sonar device

int echoPin = 7;                                // SRF05 echo pin (digital 2)
int initPin = 8;                                // SRF05 trigger pin (digital 3)
unsigned long pulseTime = 0;                    // stores the pulse in Micro Seconds
unsigned long distance = 0;                     // variable for storing the distance (cm)

// setup pins/values for LED

int redLEDPin = 3;                              // Red LED, connected to digital PWM pin 9
int redLEDValue = 0;                            // stores the value of brightness for the LED (0 = fully off, 255 = fully on)

//setup

void setup() {

  pinMode(redLEDPin, OUTPUT);                   // sets pin 9 as output
  pinMode(initPin, OUTPUT);                     // set init pin 3 as output
  pinMode(echoPin, INPUT);                      // set echo pin 2 as input

// initialize the serial port, lets you view the
 // distances being pinged if connected to computer
     Serial.begin(9600);
 } 

// execute
void loop() {
digitalWrite(initPin, HIGH);                    // send 10 microsecond pulse
delayMicroseconds(10);                  // wait 10 microseconds before turning off
digitalWrite(initPin, LOW);                     // stop sending the pulse
pulseTime = pulseIn(echoPin, HIGH);             // Look for a return pulse, it should be high as the pulse goes low-high-low
distance = pulseTime/58;   
redLEDValue = abs(255-distance);         // Distance = pulse time / 58 to convert to cm.
analogWrite(redLEDPin, redLEDValue);          // Write current value to LED pins
Serial.println(distance);         // print out the average distance to the debugger

delay(500);                                   // wait 100 milli seconds before looping again

}
