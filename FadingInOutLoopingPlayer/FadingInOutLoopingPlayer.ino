/*************************************************** 
  FadingInOut Looping Music Player for Adafruit MP3 Music Player
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


// Size of array that stores the latest sensor values for Moving-average filter algorithm
#define WINDOW_SIZE 20 // the larger the Window Size is, the more the filtered signal will lag behind the raw signal

// Sensor pins I/O on board
const int trigger = 5;
const int echo = 2;


// PROGRAM VARIABLES

int volume; // volume will range from 0 to 255 (don't forget, higher value means lower volume)
float distance;
float stime = 0;

// Moving average filter-related variables to clean up noisy sensor data
int INDEX = 0;
int SUM = 0;
int READINGS[WINDOW_SIZE];
int AVERAGED = 0;

bool doCalibrate = false;

bool inRange = false;


/*************************************************** 
  BEGIN CUSTOM SETTINGS SECTION
  Fine Tuning the Instrument Sensitivity, Distances, and Volume Variations
 ****************************************************/
 
// Default maximum sensor time for the sensor
float distanceHigh = 20000;
// Threshold Time
float thresholdTime = 500;


/*************************************************** 
  END OF CUSTOM SETTINGS SECTION
 ****************************************************/
 

// create shield object
Adafruit_VS1053_FilePlayer musicPlayer = Adafruit_VS1053_FilePlayer(SHIELD_RESET, SHIELD_CS, SHIELD_DCS, DREQ, CARDCS);
  
void setup() {
  Serial.begin(9600);

  pinMode(trigger, OUTPUT);
  pinMode(echo, INPUT);

  Serial.println("FadingInOut Looping Music Player by jleblond based on Adafruit VS1053 Player");

  if (doCalibrate) {
    calibrate();
  }

  Serial.println(distanceHigh);
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
  
  // Set volume for left, right channels. lower numbers == louder volume!
  volume = 255;  // initial volume
  musicPlayer.setVolume(volume, volume);
  
  // Play a file in the background, REQUIRES interrupts!
  Serial.println(F("Playing track 001"));
  musicPlayer.startPlayingFile("/track001.mp3");
}

void ping() {
  digitalWrite(trigger, HIGH);
  digitalWrite(trigger, LOW);

  float stime = pulseIn(echo, HIGH);

  if (stime != 0) {

    if (stime > thresholdTime) {
      inRange = true;
      
    }
    else if (stime < thresholdTime) {
      inRange = false;
      
    }
    
    Serial.println(stime);
    Serial.println(inRange);
  }
}

void loop() {
  while (musicPlayer.playingMusic) { 
    ping();

    if (inRange) {
        if (volume > 5) {
            volume = volume - 2;
        } else if (volume <= 5) {
            volume = 0;
        }
    } else {
        if (volume >= 245) {
            volume = 255;
        } else if (volume < 245) {
            volume = volume + 2;
        }
    }
    
    musicPlayer.setVolume(volume, volume);
    delay(50);
  }
  if (musicPlayer.stopped()) {
    musicPlayer.startPlayingFile("/track001.mp3");
  }
}



void calibrate() {
  // Sensor distance calibration for 5 seconds
  Serial.println("*** START CALIBRATION ***");
  while (millis() < 5000) {
    digitalWrite(trigger, HIGH);
    digitalWrite(trigger, LOW);
    distance = pulseIn(echo, HIGH);
    Serial.println(distance);

    if (distance > distanceHigh) {
      distanceHigh = distance;
    }
  }
  Serial.println("*** END CALIBRATION ***");
}
