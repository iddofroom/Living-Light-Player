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
#include "state.h"


// audio

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

// playing mode
bool interactiveMode = true;
bool playingInteractiveSong = false;

#define STATE_DEBOUNCE_TIME 1

State state, prevState = IDLE;
State lastState = IDLE;
unsigned long lastDebounceTime = 0;
unsigned long stateDebounceDelay = STATE_DEBOUNCE_TIME;
unsigned long songStartTime = 0;

unsigned long lastPlayTime = 0;
unsigned long quietDelay = 300000; // in ms, divide by 60000 for minutes

unsigned long lastMonitorTime = 0;
unsigned long MonitorDelay = 5000;

int curr_file_i = 0;
// Hidden coupling between files_iter_rr to this array
// enum State {IDLE, BACK0, BACK1, BACK2, BACK3, BACK4, BACK5, RFID, SUNRISE, SUNRISE_ACID, SUNRISE_MUSHROOM, SUNSET, SUNSET_ACID, SUNSET_MUSHROOM};
enum State back_states[] = {BACK0, BACK1, BACK2, BACK3, BACK4, BACK5};
const char *files_iter_rr[] = {"sunset1.wav", "sunset2.wav", "sunset3.wav", "sunset4.wav", "sunset5.wav", "sunset6.wav", "sunset7.wav", "sunset8.wav", 
                                "sunset9.wav", "sunset10.wav", "sunset11.wav", "sunset12.wav", "sunset13.wav"};

// buttons

#define BTN_DEBOUNCE_TIME 50
#define MAX_IO 50 //50 is the max number of IO pins

int lastBtnsState[MAX_IO]; 
int btnsState[MAX_IO];
unsigned long lastDebouncedTime[MAX_IO];

unsigned long debounceDelay = BTN_DEBOUNCE_TIME;

#define ARDUINO_RFID 33
#define BOOT_MODE_PIN 36

#define BTN_MUSHROOM 38
#define BTN_ACID 39
#define BTN_SUNSET 40
#define BTN_SUNRISE 41
#define RELAY 34

byte buttons[] = { ARDUINO_RFID, BTN_MUSHROOM, BTN_ACID, BTN_SUNSET, BTN_SUNRISE };

// Debounced check that button is indeed pressed
bool is_button_pressed(int btn_port) {
  int reading = digitalRead(btn_port);
  if (reading != lastBtnsState[btn_port]) {
    // reset the debouncing timer
    lastDebouncedTime[btn_port] = millis();
  }
  if ((millis() - lastDebouncedTime[btn_port]) > debounceDelay) {
    if (reading != btnsState[btn_port]) {
      btnsState[btn_port] = reading;
      if (btnsState[btn_port] == HIGH) {
        Serial.println("Button pressed");
        Serial.println(btn_port);
        return true;
      }
    }
  }
  lastBtnsState[btn_port] = reading;

  return false;
}

void setup_buttons() {
  for (byte i = 0; i < (sizeof(buttons)/sizeof(buttons[0])); i++) {
    pinMode(buttons[i], INPUT_PULLUP);   
  }
  
  pinMode(RELAY, OUTPUT);
  digitalWrite(RELAY, LOW);
  
  // we are initializing an array of maximum number of ports
  for (byte i = 0; i < MAX_IO; i++) {
    lastBtnsState[i] = HIGH;
    btnsState[i] = HIGH;
    lastDebouncedTime[i] = 0;
  }
}


void setup() {
    Serial.begin(115200);
    Serial.println("Serial Port Started.");
    setup_buttons();
    Serial.println("Initialized Buttons.");
    
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

    // Teensies State setup
    stateInit();

    // boot mode
    pinMode(BOOT_MODE_PIN, INPUT_PULLUP);
    if (digitalRead(BOOT_MODE_PIN) == LOW)
    {
        interactiveMode = false;
        Serial.println("Boot mode pin is LOW, mode is set to AUTO");
    } else {
        interactiveMode = true;
        Serial.println("Boot mode pin is HIGH, mode is set to INTERACTIVE");
    }
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
    Serial.println(F("Stopping file"));
    playWav1.stop();
}

void handle_rfid() {
  if (is_button_pressed(ARDUINO_RFID))
  {
    Serial.println(F("RFID reader indication is HIGH. "));
    state = RFID;
    Serial.print(F("RFID set state to: "));
    Serial.println(state);

    digitalWrite(RELAY, HIGH);          // Shut off relay when user selects song, TODO: verify relay on/off settings
    playFile(files_iter_rr[state-1]);
    playingInteractiveSong = true;
  }
}


void loop() {
    if (!playingInteractiveSong){
        // Check for user request from buttons, do we want to handle case of SUNRISE+SUNSET pressed? currently priority is given to SUNRISE
        if (is_button_pressed(BTN_SUNRISE)) {
            if (is_button_pressed(BTN_ACID)) {
                Serial.println("SUNRISE button pressed with ACID select");
                state = SUNRISE_ACID;
            } else if (is_button_pressed(BTN_MUSHROOM)) {
                Serial.println("SUNRISE button pressed with MUSHROOM select");
                state = SUNRISE_MUSHROOM;
            } else {
                Serial.println("SUNRISE button pressed with no select");
                state = SUNRISE;
            }
            digitalWrite(RELAY, HIGH);          // Shut off relay when user selects song, TODO: verify relay on/off settings
            playFile(files_iter_rr[state-1]);
            playingInteractiveSong = true;
        } else if (is_button_pressed(BTN_SUNSET)) {
            if (is_button_pressed(BTN_ACID)) {
                Serial.println("SUNSET button pressed with ACID select");
                state = SUNSET_ACID;
            } else if (is_button_pressed(BTN_MUSHROOM)) {
                Serial.println("SUNSET button pressed with MUSHROOM select");
                state = SUNSET_MUSHROOM;
            } else {
                Serial.println("SUNSET button pressed with no select");
                state = SUNSET;
            }
            digitalWrite(RELAY, HIGH);          // Shut off relay when user selects song, TODO: verify relay on/off settings
            playFile(files_iter_rr[state-1]);
            playingInteractiveSong = true;
        }

        // RFID indication handling
        handle_rfid();
    }

    // If no file is playing for more than the quietDelay set, play next background file
    if (!playWav1.isPlaying()) {
        playingInteractiveSong = false; // reallow interaction if no song is playing
        if((millis() - lastPlayTime) > quietDelay) {
            if (!interactiveMode) {     // non interactive mode rotates on background states
                state = back_states[curr_file_i];
                curr_file_i = (curr_file_i + 1) % (sizeof(back_states) / sizeof(back_states[0]));
            } else {
                state = BACK5; // TODO: what is the background state for interactive mode?
            }
            Serial.print("No file is playing, loading new file number: ");
            Serial.println(files_iter_rr[state - 1]);
            playFile(files_iter_rr[state-1]);
            lastPlayTime = millis();
        }
    } else {
        lastPlayTime = millis();
    }

    // State tracking between two teensies
    if (state != prevState)
    {
        songStartTime = millis();
        stateEncode(state);
    }
    prevState = state;
    // Holding non IDLE state for a short while so we can use debounce on second teensy to capture state safely
    if (state != IDLE)
    {
        if ((millis() - songStartTime) > stateDebounceDelay)
        {
            state = IDLE;
        }
    }

    // Monitoring, not a must, just for debugging
    if((millis() - lastMonitorTime) > MonitorDelay) {
        Serial.print("Sunset Audio: ");
        if (playWav1.isPlaying()) {
            Serial.print("Current playing audio file is: "); Serial.println(files_iter_rr[prevState-1]);
        } else {
            Serial.println("NO audio file playing");
        }
        lastMonitorTime = millis();
    }
}
