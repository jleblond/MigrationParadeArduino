/*************************************************** 
  Sensor Distance Controller Player for Adafruit MP3 Music Player
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
float distance;
float stime = 0;
int fileNumber;

// Moving average filter-related variables to clean up noisy sensor data
int INDEX = 0;
int SUM = 0;
int READINGS[WINDOW_SIZE];
int AVERAGED = 0;


/*************************************************** 
  BEGIN CUSTOM SETTINGS SECTION
  Fine Tuning the Instrument Sensitivity, Distances, and Volume Variations
 ****************************************************/
 
// Default maximum sensor time for the sensor
float stimeHigh = 20000;

int volume = 0; // volume will range from 0 to 255 (don't forget, higher value means lower volume)


/*************************************************** 
  END OF CUSTOM SETTINGS SECTION
 ****************************************************/
//char *filesArray[10];

const int maxSize=15;
char file0[maxSize] = "/track001.mp3";
char file1[maxSize] = "/track002.mp3";
char file2[maxSize] = "/track003.mp3";
char file3[maxSize] = "/track004.mp3";
char file4[maxSize] = "/track005.mp3";
char file5[maxSize] = "/track006.mp3";
char file6[maxSize] = "/track007.mp3";
char file7[maxSize] = "/track008.mp3";
char file8[maxSize] = "/track009.mp3";
char file9[maxSize] = "/track010.mp3";

char *filesArray[] = {file0, file1, file2, file3, file4, file5, file6, file7, file8, file9};
 

// create shield object
Adafruit_VS1053_FilePlayer musicPlayer = Adafruit_VS1053_FilePlayer(SHIELD_RESET, SHIELD_CS, SHIELD_DCS, DREQ, CARDCS);

float distanceFromSensorTime(float sensorTime) {
  /**
   * Formula: d = v*t
   * Distance (d) : distance in meter (m)
   * Time (t): The sensor measures the time from the moment the wave being sent to moment the wave returns to the sensor. Since we are only interested in the distance between and the object we need to halve our time by divide by 2.
   * Speed (v): Sonic moves by 343, 2 meters per second (m/s). We have to adjust the unit of Speed. From meters per second to centimetre per microsecond, because the microcontroller measures time in microseconds.
   * 343, 2 meter per second (m/s) = 0, 03432 centimeter per microsecond (cm/μs)
   * 
   * ref. for formula: https://create.arduino.cc/projecthub/pericardium/ultrasonic-distance-sensor-373036
   **/

   return 0.03432 * (sensorTime/2);
}

float distanceHigh = distanceFromSensorTime(stimeHigh);
  
void setup() {
  Serial.begin(9600);

  pinMode(trigger, OUTPUT);
  pinMode(echo, INPUT);

  Serial.println("Sensor Distance Controller Player by jleblond based on Adafruit VS1053 Player");

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

  musicPlayer.setVolume(volume, volume);
}

void ping() {
  digitalWrite(trigger, HIGH);
  digitalWrite(trigger, LOW);

  float stime = pulseIn(echo, HIGH);

  if (stime != 0) {
    distance = distanceFromSensorTime(stime);
    int val = (distance/distanceHigh * 10);
    int averageVal = movingAverage(val);
    
    if (averageVal >= 0 && averageVal  < 10) {
      int tempFileNumber = averageVal;
      if (tempFileNumber != fileNumber) {
        fileNumber = tempFileNumber;
        char *fileName = filesArray[fileNumber];
        Serial.println(fileNumber);
        if (musicPlayer.stopped() && fileNumber <= 9) {
          musicPlayer.startPlayingFile(fileName);
        }
      }
    }
    
  }
}

void loop() {
  ping();
  delay(10);
}

int movingAverage(int readingVal) {
  // Moving average filter to clean up noisy sensor data
  // ref. for algorithm: https://maker.pro/arduino/tutorial/how-to-clean-up-noisy-sensor-data-with-a-moving-average-filter
  SUM = SUM - READINGS[INDEX];       // Remove the oldest entry from the sum
  READINGS[INDEX] = readingVal;           // Add the newest reading to the window
  SUM = SUM + readingVal;                 // Add the newest reading to the sum
  INDEX = (INDEX+1) % WINDOW_SIZE;   // Increment the index, and wrap to 0 if it exceeds the window size
  AVERAGED = SUM / WINDOW_SIZE;      // Divide the sum of the window by the window size for the result

  return AVERAGED;
}
