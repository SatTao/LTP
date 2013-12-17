// RGB control via serial from laptop for a 12V led string

// This actually runs at 7.4v roughly, powered by a LiPoly

// 10cm of strip maximally draws 40mA at this voltage.

// 2N2222 transistors have a maximum rated current of 1A steady,
// so we can afford to have about 7.5 m of strip if needs be.

// Designed for use with 1m or less.

int red = 5;
int blue = 6;
int green = 9; // PWM outputs to transistors.

byte RED[] = {255,0,0};
byte GREEN[] = {0,255,0};
byte BLUE[] = {0,0,255};
byte PURPLE[] = {255,0,255};
byte TURQUOISE[] = {0,255,255};
byte YELLOW[] = {255,255,0};
byte WHITE[] = {255,255,255};
byte BLACK[] = {0,0,0};

byte current_colour[] = {0,0,0};

void setup(){
  pinMode(red, OUTPUT); 
  pinMode(green, OUTPUT);
  pinMode(blue, OUTPUT);
  
  Serial.begin(9600);
  delay(1000);
  Serial.println("START");
}

void loop(){
  
 crossfade(BLACK, WHITE, 200, 3000);
 pulse(WHITE, BLUE, 200, 750);
 pulse(WHITE, BLUE, 200, 750);
 pulse(WHITE, RED, 200, 750);
 pulse(WHITE, RED, 200, 750);
 pulse(WHITE, GREEN, 200, 750);
 pulse(WHITE, GREEN, 200, 750);
 pulse(WHITE, PURPLE, 200, 750);
 pulse(WHITE, PURPLE, 200, 750);
 pulse(WHITE, YELLOW, 200, 750);
 pulse(WHITE, YELLOW, 200, 750);
 pulse(WHITE, TURQUOISE, 200, 750);
 pulse(WHITE, TURQUOISE, 200, 750);
 crossfade(WHITE, BLACK, 200, 3000);
 delay(1000);
}

void setcol(byte *col){
  analogWrite(red, col[0]);
  analogWrite(green, col[1]);
  analogWrite(blue, col[2]);
}

boolean crossfade(byte *from, byte *to, int steps, int ms){
  
  // Fade from one colour (defaults to the current one) to a new colour in steps in ms (ms must be larger than steps). 
  
  if (ms < steps) {return false;}
  
  byte set[] = {0,0,0};set[0]=from[0];set[1]=from[1];set[2]=from[2];
  int ms_per_step = floor(ms/steps);
  byte red_bits_per_step = (byte)floor((to[0]-from[0])/steps);
  byte green_bits_per_step = (byte)floor((to[1]-from[1])/steps);
  byte blue_bits_per_step = (byte)floor((to[2]-from[2])/steps);
  
  for(int i=0;i<steps;i++){    
    set[0] = set[0]+red_bits_per_step;
    set[1] = set[1]+green_bits_per_step;
    set[2] = set[2]+blue_bits_per_step;
    setcol(set);
    delay(ms_per_step);
  }
  
  current_colour[0]=to[0];
  current_colour[1]=to[1];
  current_colour[2]=to[2];
  setcol(current_colour);
  
  return true;
}

boolean pulse(byte *from, byte *to, int steps, int ms){
  
  if (floor(ms/2) < steps) {return false;}
  
  crossfade(from, to, steps, floor(ms/2));
  crossfade(to, from, steps, floor(ms/2));
  return true;
}
  
  
