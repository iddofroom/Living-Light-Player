// Simple WAV file player example
//
// Three types of output may be used, by configuring the code below.
//
//   1: Digital I2S - Normally used with the audio shield:
//         http://www.pjrc.com/store/teensy3_audio.html
//
//   2: Digital S/PDIF - Connect pin 22 to a S/PDIF transmitter
//         https://www.oshpark.com/shared_projects/KcDBKHta
//
//   3: Analog DAC - Connect the DAC pin to an amplified speaker
//         http://www.pjrc.com/teensy/gui/?info=AudioOutputAnalog
//
// To configure the output type, first uncomment one of the three
// output objects.  If not using the audio shield, comment out
// the sgtl5000_1 lines in setup(), so it does not wait forever
// trying to configure the SGTL5000 codec chip.
//
// The SD card may connect to different pins, depending on the
// hardware you are using.  Uncomment or configure the SD card
// pins to match your hardware.
//
// Data files to put on your SD card can be downloaded here:
//   http://www.pjrc.com/teensy/td_libs_AudioDataFiles.html
//
// This example code is in the public domain.

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

AudioPlaySdWav           playWav1;
// Use one of these 3 output types: Digital I2S, Digital S/PDIF, or Analog DAC
AudioOutputI2S           audioOutput;
//AudioOutputSPDIF       audioOutput;
// AudioOutputAnalog      audioOutput;
//On Teensy LC, use this for the Teensy Audio Shield:
//AudioOutputI2Sslave    audioOutput;

AudioConnection          patchCord1(playWav1, 0, audioOutput, 0);
AudioConnection          patchCord2(playWav1, 1, audioOutput, 1);
AudioControlSGTL5000     sgtl5000_1;

// Use these with the Teensy Audio Shield
#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  7
#define SDCARD_SCK_PIN   14

// Use these with the Teensy 3.5 & 3.6 SD card
// #define SDCARD_CS_PIN    BUILTIN_SDCARD
// #define SDCARD_MOSI_PIN  11  // not actually used
// #define SDCARD_SCK_PIN   13  // not actually used

// Use these for the SD+Wiz820 or other adaptors
//#define SDCARD_CS_PIN    4
//#define SDCARD_MOSI_PIN  11
//#define SDCARD_SCK_PIN   13

#define GPIO0 29
#define GPIO1 30
#define GPIO2 31
#define GPIO3 32
#define DEBOUNCE_TIME 1

u_int8_t state, prevState = 0;
u_int8_t lastState = 0;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = DEBOUNCE_TIME;
u_int8_t reading;

// unsigned long lastPlayTime = 0;
// unsigned long quietDelay = 5000;

unsigned long lastMonitorTime = 0;
unsigned long MonitorDelay = 5000;

const char *files_iter_rr[] = {"silence.wav", "silence.wav", "silence.wav", "silence.wav", "silence.wav", "silence.wav", "silence.wav", "silence.wav", "silence.wav", "silence.wav", "chip.wav"};

void setup() {
  Serial.begin(115200);

  // Audio connections require memory to work.  For more
  // detailed information, see the MemoryAndCpuUsage example
  AudioMemory(8);

  // Comment these out if not using the audio adaptor board.
  // This may wait forever if the SDA & SCL pins lack
  // pullup resistors
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.7);

  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
  if (!(SD.begin(SDCARD_CS_PIN))) {
    // stop here, but print a message repetitively
    while (1) {
      Serial.println("Unable to access the SD card, fix and press reset button");
      delay(500);
    }
  }
  Serial.println("Audio setup done.");

  // Teensies interface IOs setup
  pinMode(GPIO0, INPUT);
  pinMode(GPIO1, INPUT);
  pinMode(GPIO2, INPUT);
  pinMode(GPIO3, INPUT);
  Serial.print("GPIO pins set to: "); Serial.print(GPIO3); Serial.print(" "); Serial.print(GPIO2); Serial.print(" "); Serial.print(GPIO1); Serial.print(" "); Serial.println(GPIO0);
}

void playFile(const char *filename)
{
  Serial.print("Playing file: ");
  Serial.println(filename);

  // Start playing the file.  This sketch continues to
  // run while the file plays.
  playWav1.play(filename);

  // A brief delay for the library read WAV info
  // delay(25);

  // Simply wait for the file to finish playing.
  // while (playWav1.isPlaying()) {
    // uncomment these lines if you audio shield
    // has the optional volume pot soldered
    //float vol = analogRead(15);
    //vol = vol / 1024;
    // sgtl5000_1.volume(vol);
  // }
}

void stopFile()
{
  Serial.println("Stopping file");
  playWav1.stop();
}

void loop() {
  // Read state from GPIOs and encode
  reading = (digitalRead(GPIO3) * 8) + (digitalRead(GPIO2) * 4) + (digitalRead(GPIO1) * 2) + digitalRead(GPIO0);
  if (reading != lastState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != state) {
      prevState = state;
      state = reading;
      Serial.print("New state received: "); Serial.println(state);
      switch (state) {
        case 0: // default state that does nothing is required, do not use
          // playFile("cave1.wav");  // filenames are always uppercase 8.3 format
          break;
        case 1 ... (sizeof(files_iter_rr) / sizeof(files_iter_rr[0])):
          playFile(files_iter_rr[state-1]);
          break;
        case 15:
          stopFile();
          break;
        default: // state can go up to 15
          playFile("amir.wav");
          break;
      }
    }
  }
  lastState = reading;

  // If no file is playing, play default background music, removing for now so all plays are only triggered
  // if (!playWav1.isPlaying()) {
  //   if((millis() - lastPlayTime) > quietDelay) {
  //     playFile("cave2.wav");
  //     lastPlayTime = millis();
  //   }
  // }
  if((millis() - lastMonitorTime) > MonitorDelay) {
    Serial.print("COW Audio: ");
    if (playWav1.isPlaying()) {
      Serial.print("Current playing audio file is: "); Serial.println(files_iter_rr[prevState-1]);
    } else {
      Serial.println("NO audio file playing");
    }
    lastMonitorTime = millis();
  }
}
