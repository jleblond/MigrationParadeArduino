/*************************************************** 
  Looping Music Player for Adafruit MP3 Music Player
  Written by jleblond


  Based on Adafruit VS1053 Codec Breakout by Limor Fried/Ladyada for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 ****************************************************/
 

// include SPI, MP3 and SD libraries
#include <SPI.h>
#include <Adafruit_VS1053.h>
#include <SD.h>


// These are the pins used for the music maker shield
#define SHIELD_RESET  -1      // VS1053 reset pin (unused!)
#define SHIELD_CS     7      // VS1053 chip select pin (output)
#define SHIELD_DCS    6      // VS1053 Data/command select pin (output)

// These are common pins between breakout and shield
#define CARDCS 4     // Card chip select pin
// DREQ should be an Int pin, see http://arduino.cc/en/Reference/attachInterrupt
#define DREQ 3       // VS1053 Data request, ideally an Interrupt pi


// create shield object
Adafruit_VS1053_FilePlayer musicPlayer = Adafruit_VS1053_FilePlayer(SHIELD_RESET, SHIELD_CS, SHIELD_DCS, DREQ, CARDCS);

int volume = 0; // volume will range from 0 to 255 (don't forget, higher value means lower volume)
  
void setup() {
  Serial.begin(9600);

  Serial.println("Looping Music Player by jleblond based on Adafruit VS1053 Player");

  Serial.println("*******START**********");

  if (! musicPlayer.begin()) { // initialise the music player
     Serial.println(F("Couldn't find VS1053, do you have the right pins defined?"));
     while (1);
  }
  Serial.println(F("VS1053 found"));
  
   if (!SD.begin(CARDCS)) {
    Serial.println(F("SD failed, or not present"));
    while (1);
  }

  // If DREQ is on an interrupt pin (on uno, #2 or #3) we can do background
  // audio playing
  if (!musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT)) {
    Serial.println("DREQ pin not irq");
    while (1);
  }
  
  // Play a file in the background, REQUIRES interrupts!
  musicPlayer.setVolume(volume, volume);
  Serial.println(F("Playing track 001"));
  musicPlayer.startPlayingFile("/track001.mp3");
}


void loop() {
  if (musicPlayer.stopped()) {
    musicPlayer.startPlayingFile("/track001.mp3");
  }
}
