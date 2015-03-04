// MusicIC_x.ino

// Music IC implementation using alternative library for sd card playback. May be a better solution.

#include <SD.h>                      // need to include the SD library
//#define SD_ChipSelectPin 53  //example uses hardware SS pin 53 on Mega2560
#define SD_ChipSelectPin 10  //using digital pin 4 on arduino nano 328, can use other pins
#include <TMRpcm.h>           //  also need to include this library...
#include <SPI.h>

TMRpcm tmrpcm;   // create an object for use in this sketch

// NOTE BENE: pcmCobfig.h has been edited; sd card half speed is selected, output 2 on pin 10 is disabled, tag handling is enabled.

void setup(){

  tmrpcm.speakerPin = 9; //5,6,11 or 46 on Mega, 9 on Uno, Nano, etc

  Serial.begin(9600);
  Serial.println("I'm awake!");
  if (!SD.begin(SD_ChipSelectPin)) {  // see if the card is present and can be initialized:
    Serial.println("SD fail");  
    return;   // don't do anything more if not
  }
  else { Serial.println("OK, SD card is alive");}
  Serial.println("Playing");
  tmrpcm.play("ADMIN.wav"); //the sound file "music" will play each time the arduino powers up, or is reset
  Serial.println("Done");
}



void loop(){  

  if(Serial.available()){    
    switch(Serial.read()){ 
      case 'p':
        Serial.println("Playing");
        tmrpcm.play("ADMIN.wav");
        Serial.println("Done");
        break;
      case 's':
        Serial.println("Stopping - timer still on");
        tmrpcm.stopPlayback();
        Serial.println("Done");
        break;
      case 'S':
        Serial.println("Stopping - timer disabled");
        tmrpcm.disable();
        Serial.println("Done");
        break;
      case 'q':
        Serial.println("Low quality");
        tmrpcm.quality(0);
        Serial.println("Done");
        break;
      case 'Q':
        Serial.println("High quality");
        tmrpcm.quality(1);
        Serial.println("Done");
        break;
      case '+':
        Serial.println("Volume up");
        tmrpcm.volume(1);
        Serial.println("Done");
        break;
      case '-':
        Serial.println("Volume down");
        tmrpcm.volume(0);
        Serial.println("Done");
        break;
      case 'm':
        Serial.println("Pause/unpause");
        tmrpcm.pause();
        Serial.println("Done");
        break;
      default: break;
    }
  }

}
