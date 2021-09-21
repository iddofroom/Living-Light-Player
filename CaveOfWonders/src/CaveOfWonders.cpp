#include "Adafruit_VL53L0X.h"
#include "SdLedsPlayer.h"

#define LEDS_PER_STRIP 254
#define FILE_TO_PLAY "under"
#define RANGE_BOOT_RETRIES 10
#define TOF_MEAS_INTERVAL 40
#define KEYPIN 22
#define DEBOUNCE_TIME 50
#define RELAYPIN1 1
#define RELAYPIN2 23
#define OUTGPIO0 29
#define OUTGPIO1 30
#define OUTGPIO2 31
#define OUTGPIO3 32

int curr_file_i = 0;
const char *files_iter_rr[] = {"cave1", "cave2", "cave3", "cave4"};

/*
 * SdLedsPlayer is the class that handles reading frames from file on SD card,
 * and writing it to the leds.
 * It needs to be initialized with LEDS_PER_STRIP (must match the leds per strips
 * of the file written to SD card).
 * It also needs to recive the leds buffer for OctoWS2811, should be initialized as follows
 */
DMAMEM int display_memory[LEDS_PER_STRIP * 6]; 
int drawing_memory[LEDS_PER_STRIP * 6];
SdLedsPlayer sd_leds_player(LEDS_PER_STRIP, display_memory, drawing_memory);
unsigned long frame_timestamp;
uint8_t brightness = 50; // range is 0 (off) to 255 (full brightness)

// Range sensor
Adafruit_VL53L0X lox = Adafruit_VL53L0X();
bool rangeBooted = false;
int rangeBootCnt = 0;
int range = -1;
int rangeAccum = -1;
bool rangeActive = true; 
int rangeCnt = 0;

// Key Switch
int keyState = HIGH;
int lastKeyState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = DEBOUNCE_TIME;
bool keyActive = true;
int reading;

// Song tracking
uint8_t state, prevState = 0;
unsigned long currSongTime = 0, songStartTime = 0, lastRangeTime = 0, procTime = 0;

// Monitoring vars
unsigned long lastMonitorTime = 0;
unsigned long MonitorDelay = 5000;

void stateEncode (uint8_t state) {
  Serial.print("Changing to state: "); Serial.println(state);
  switch (state) {
    // default state that does nothing is required, do not use
    case 0:
      digitalWrite(OUTGPIO0, LOW);
      digitalWrite(OUTGPIO1, LOW);
      digitalWrite(OUTGPIO2, LOW);
      digitalWrite(OUTGPIO3, LOW);
      // relays ON
      digitalWrite(RELAYPIN1, HIGH); 
      digitalWrite(RELAYPIN2, HIGH); 
      break;

    // Background index 0
    case 1:
      digitalWrite(OUTGPIO0, HIGH);
      digitalWrite(OUTGPIO1, LOW);
      digitalWrite(OUTGPIO2, LOW);
      digitalWrite(OUTGPIO3, LOW);
      // relays ON
      digitalWrite(RELAYPIN1, HIGH); 
      digitalWrite(RELAYPIN2, LOW); 
      break;

    // Background index 1
    case 2:
      digitalWrite(OUTGPIO0, LOW);
      digitalWrite(OUTGPIO1, HIGH);
      digitalWrite(OUTGPIO2, LOW);
      digitalWrite(OUTGPIO3, LOW);
      // relays ON
      digitalWrite(RELAYPIN1, LOW); 
      digitalWrite(RELAYPIN2, HIGH); 
      break;

    // Background index 2
    case 3:
      digitalWrite(OUTGPIO0, HIGH);
      digitalWrite(OUTGPIO1, HIGH);
      digitalWrite(OUTGPIO2, LOW);
      digitalWrite(OUTGPIO3, LOW);
      // shut off relays
      digitalWrite(RELAYPIN1, HIGH); 
      digitalWrite(RELAYPIN2, HIGH); 
      break;

    // Background index 3
    case 4:
      digitalWrite(OUTGPIO0, LOW);
      digitalWrite(OUTGPIO1, LOW);
      digitalWrite(OUTGPIO2, HIGH);
      digitalWrite(OUTGPIO3, LOW);
      // shutting off RELAYS
      digitalWrite(RELAYPIN1, LOW); // shutting off RELAYS
      digitalWrite(RELAYPIN2, LOW); // shutting off RELAYS
      break;

    // range sensor triggered
    case 5:
      digitalWrite(OUTGPIO0, HIGH);
      digitalWrite(OUTGPIO1, LOW);
      digitalWrite(OUTGPIO2, HIGH);
      digitalWrite(OUTGPIO3, LOW);
      break;

    // key triggered
    case 6:
      digitalWrite(OUTGPIO0, LOW);
      digitalWrite(OUTGPIO1, HIGH);
      digitalWrite(OUTGPIO2, HIGH);
      digitalWrite(OUTGPIO3, LOW);
      break;

    // unassigned state
    case 7:
      digitalWrite(OUTGPIO0, HIGH);
      digitalWrite(OUTGPIO1, HIGH);
      digitalWrite(OUTGPIO2, HIGH);
      digitalWrite(OUTGPIO3, LOW);
      break;

    default:
      digitalWrite(OUTGPIO0, LOW);
      digitalWrite(OUTGPIO1, LOW);
      digitalWrite(OUTGPIO2, LOW);
      digitalWrite(OUTGPIO3, LOW);
  }
}

void setup() {
  Serial.begin(115200);
  // wait until serial port opens for native USB devices
  // while (! Serial) {
  //   delay(1);
  // }
  Serial.println("Serial Port Started.");
  sd_leds_player.setup();
  sd_leds_player.setBrightness(brightness);
  
  // Key switch setup
  pinMode(KEYPIN, INPUT_PULLUP);
  Serial.print("GPIO pin for key switch set to: "); Serial.println(KEYPIN);

  // Relay setup
  pinMode(RELAYPIN1, OUTPUT);
  pinMode(RELAYPIN2, OUTPUT);
  Serial.print("Relay pins set to: "); Serial.print(RELAYPIN1); Serial.print(" "); Serial.println(RELAYPIN2);

  // output GPIO setup
  pinMode(OUTGPIO0, OUTPUT);
  pinMode(OUTGPIO1, OUTPUT);
  pinMode(OUTGPIO2, OUTPUT);
  pinMode(OUTGPIO3, OUTPUT);
  stateEncode(state);
  Serial.print("State output pins set to: "); Serial.print(OUTGPIO0); Serial.print(" "); Serial.print(OUTGPIO1); Serial.print(" "); Serial.print(OUTGPIO2); Serial.print(" "); Serial.println(OUTGPIO3);

  // TOF sensor setup
  Serial.println("Starting VL53L0X boot");
  while ((!rangeBooted) && (rangeBootCnt < RANGE_BOOT_RETRIES)) {
    if (lox.begin()) {
      Serial.print(F("VL53L0X sensor boot done successfully after ")); Serial.print(rangeBootCnt); Serial.println(" attempts.");
      rangeBooted = true;
    }
    else {
      Serial.print(F("Failed to boot VL53L0X, retrying.. ")); Serial.println(rangeBootCnt);
      rangeBootCnt++;
      delay(1000);
    }
  }
  if (rangeBooted) {
    if (!lox.startRangeContinuous(TOF_MEAS_INTERVAL)){
      Serial.println(F("Failed to start VL53L0X continuous ranging\n"));
    }
    Serial.println(F("VL53L0X sensor started in continuous ranging mode.\n"));
  }

  // RFID reader setup

}

void loop() {

  if (rangeBooted) {
    if (lox.isRangeComplete()) {   // TOF sensor read when measurement data is available
      range = lox.readRangeResult();
      // Serial.print(millis());Serial.print(" : Distance (mm): "); Serial.println(range);
      rangeCnt++;
      rangeAccum += range;
      if (rangeCnt >= 3) { // number of measurements to average
        range = rangeAccum / rangeCnt;
        rangeAccum = 0;
        rangeCnt = 0;
        // Brightness by range
        if (range > 200) { // starting from what range do we want to play with Brightness?
          sd_leds_player.setBrightness(brightness); // default brightness, consider value
          rangeActive = false; // disabling activation of song by range sensor by jumping from long distance
        }
        else if (range <= 200 && range > 40) {
          sd_leds_player.setBrightness(brightness+200-range);
          rangeActive = true; // enabling activation of song by range sensor from short distance
        }
        else if ((range < 40) && rangeActive && !keyActive) { // don't allow range sensor song triggering from long distance or key switch triggered
          Serial.print("range sensor triggered at distance (mm): "); Serial.println(range);
          sd_leds_player.load_file(FILE_TO_PLAY);
          // sd_leds_player.setBrightness(255); // set brightness for song
          rangeActive = false;
          keyActive = true; // using the keyActive to make sure the range sensor can't trigger during its own song until next background song
          state = 5;
          songStartTime = millis();
          frame_timestamp = sd_leds_player.load_next_frame();
        }
      }
    }
  }

  // Key switch reading with debounce
  reading = digitalRead(KEYPIN);
  if (reading != lastKeyState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != keyState) {
      keyState = reading;
      if (keyState == HIGH) {
        Serial.println("key switch triggered! loading LEDS file");
        sd_leds_player.load_file(FILE_TO_PLAY);
        state = 6;
        keyActive = true;
        songStartTime = millis();
        frame_timestamp = sd_leds_player.load_next_frame();
      }
    }
  }
  lastKeyState = reading;

  if(!sd_leds_player.is_file_playing()) {
    Serial.print("No file is playing, loading new file: "); Serial.println(files_iter_rr[curr_file_i]);
    sd_leds_player.load_file(files_iter_rr[curr_file_i]);
    state = curr_file_i+1;
    curr_file_i = (curr_file_i + 1) % (sizeof(files_iter_rr) / sizeof(files_iter_rr[0]));
    keyActive = false;
    songStartTime = millis();
    frame_timestamp = sd_leds_player.load_next_frame();
  }

  currSongTime = millis() - songStartTime;
  if (currSongTime >= frame_timestamp) {
    // if ((currSongTime-frame_timestamp) != 0) {
    //   Serial.println(currSongTime-frame_timestamp);
    // }
    sd_leds_player.show_next_frame();
    frame_timestamp = sd_leds_player.load_next_frame();
  }

  if (state != prevState) {
    stateEncode(state);
  }
  prevState = state;
  // restore state to default
  state = 0;

  if((millis() - lastMonitorTime) > MonitorDelay) {
    Serial.println("COW Leds Alive");
    Serial.print("rangeActive: "); Serial.println(rangeActive ? "true" : "false");
    Serial.print("keyActive: "); Serial.println(keyActive ? "true" : "false");
    lastMonitorTime = millis();
  }
}
