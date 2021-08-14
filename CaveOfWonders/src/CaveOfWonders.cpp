#include "Adafruit_VL53L0X.h"
#include "SdLedsPlayer.h"

#define LEDS_PER_STRIP 251
#define FILE_TO_PLAY "under"
#define TOF_MEAS_INTERVAL 40
#define KEYPIN 22
#define DEBOUNCE_TIME 50
#define RELAYPIN1 1
#define RELAYPIN2 23
#define OUTGPIO1 30
#define OUTGPIO2 31
#define OUTGPIO3 32

int curr_file_i = 0;
const char *files_iter_rr[] = {"under", "under"};

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
uint8_t brightness = 30; // range is 0 (off) to 255 (full brightness)

Adafruit_VL53L0X lox = Adafruit_VL53L0X();
int range = -1;
int rangeAccum = -1;
bool rangeActive = false; // temp change to disable range sensor activating song
int rangeCnt = 0;

int keyState = HIGH;
int lastKeyState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = DEBOUNCE_TIME;
bool keyActive = true;
int reading;

unsigned long currSongTime = 0, songStartTime = 0, lastRangeTime = 0, procTime = 0;

void stateEncode (uint8_t state) {
  // Serial.println(state);
  switch (state) {
    // Background index 0
    case 0:
      digitalWrite(OUTGPIO1, LOW);
      digitalWrite(OUTGPIO2, LOW);
      digitalWrite(OUTGPIO3, LOW);
      // relays ON
      digitalWrite(RELAYPIN1, HIGH); 
      digitalWrite(RELAYPIN2, HIGH); 
      break;

    // Background index 1
    case 1:
      digitalWrite(OUTGPIO1, HIGH);
      digitalWrite(OUTGPIO2, LOW);
      digitalWrite(OUTGPIO3, LOW);
      // relays ON
      digitalWrite(RELAYPIN1, HIGH); 
      digitalWrite(RELAYPIN2, LOW); 
      break;

    // Background index 2?
    case 2:
      digitalWrite(OUTGPIO1, LOW);
      digitalWrite(OUTGPIO2, HIGH);
      digitalWrite(OUTGPIO3, LOW);
      // relays ON
      digitalWrite(RELAYPIN1, LOW); 
      digitalWrite(RELAYPIN2, HIGH); 
      break;

    // range sensor triggered
    case 3:
      digitalWrite(OUTGPIO1, HIGH);
      digitalWrite(OUTGPIO2, HIGH);
      digitalWrite(OUTGPIO3, LOW);
      // shut off relays
      digitalWrite(RELAYPIN1, HIGH); 
      digitalWrite(RELAYPIN2, HIGH); 
      break;

    // key triggered
    case 4:
      digitalWrite(OUTGPIO1, LOW);
      digitalWrite(OUTGPIO2, LOW);
      digitalWrite(OUTGPIO3, HIGH);
      // shutting off RELAYS
      digitalWrite(RELAYPIN1, LOW); // shutting off RELAYS
      digitalWrite(RELAYPIN2, LOW); // shutting off RELAYS
      break;
    // unassigned state
    case 5:
      digitalWrite(OUTGPIO1, HIGH);
      digitalWrite(OUTGPIO2, LOW);
      digitalWrite(OUTGPIO3, HIGH);
      break;
    // unassigned state
    case 6:
      digitalWrite(OUTGPIO1, LOW);
      digitalWrite(OUTGPIO2, HIGH);
      digitalWrite(OUTGPIO3, HIGH);
      break;
    // unassigned state
    case 7:
      digitalWrite(OUTGPIO1, HIGH);
      digitalWrite(OUTGPIO2, HIGH);
      digitalWrite(OUTGPIO3, HIGH);
      break;

    default:
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
  
  // TOF sensor setup
  Serial.println("Starting VL53L0X boot");
  while (!lox.begin()) {
    Serial.println(F("Failed to boot VL53L0X"));
    delay(1000);
  }
  Serial.println(F("VL53L0X sensor boot done.\n"));
  if (!lox.startRangeContinuous(TOF_MEAS_INTERVAL)){
    Serial.println(F("Failed to start VL53L0X continuous ranging\n"));
  }
  Serial.println(F("VL53L0X sensor started in continuous ranging mode.\n"));
  
  // Key switch setup
  pinMode(KEYPIN, INPUT_PULLUP);
  Serial.print("GPIO pin for key switch set to: "); Serial.println(KEYPIN);

  // Relay setup
  pinMode(RELAYPIN1, OUTPUT);
  pinMode(RELAYPIN2, OUTPUT);
  Serial.print("Relay pins set to: "); Serial.print(RELAYPIN1); Serial.print(" "); Serial.println(RELAYPIN2);

  // output GPIO setup
  pinMode(OUTGPIO1, OUTPUT);
  pinMode(OUTGPIO2, OUTPUT);
  pinMode(OUTGPIO3, OUTPUT);
  stateEncode(0);
  Serial.print("State output pins set to: "); Serial.print(OUTGPIO1); Serial.print(" "); Serial.print(OUTGPIO2); Serial.print(" "); Serial.println(OUTGPIO3);
}

void loop() {

  // TOF sensor read when measurement data is available
  if (lox.isRangeComplete()) {
    range = lox.readRangeResult();
    // Serial.print(millis());Serial.print(" : Distance (mm): "); Serial.println(range); // WHY DOES IT NOT WORK WITHOUT THIS?????
    rangeCnt++;
    rangeAccum += range;
    if (rangeCnt >= 3) { // number of measurements to average
      range = rangeAccum / rangeCnt;
      rangeAccum = 0;
      rangeCnt = 0;
      // Brightness by range
      if (range > 220) { // starting from what range do we want to play with Brightness?
        sd_leds_player.setBrightness(brightness); // default brightness, consider value
      }
      else if (range <= 200 && range > 40) {
        sd_leds_player.setBrightness(brightness+200-range);
      }
      else if ((range < 40) && rangeActive) {
        Serial.print("range sensor triggered at distance (mm): "); Serial.println(range);
        sd_leds_player.load_file(FILE_TO_PLAY);
        // sd_leds_player.setBrightness(255); // set brightness for song
        rangeActive = false;
        keyActive = false;
        stateEncode(3);
        frame_timestamp = sd_leds_player.load_next_frame();
      }
    }
  }

  // Key switch reading with debounce
  if (keyActive) {
    reading = digitalRead(KEYPIN);
  }
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
        stateEncode(4);
        rangeActive = false;
        keyActive = true; // temp change for key to not disable more key presses
        songStartTime = millis();
        frame_timestamp = sd_leds_player.load_next_frame();
      }
    }
  }
  lastKeyState = reading;

  if(!sd_leds_player.is_file_playing()) {
    Serial.println("No file is playing, loading new file");
    sd_leds_player.load_file(files_iter_rr[curr_file_i]);
    stateEncode(curr_file_i+1);
    curr_file_i = (curr_file_i + 1) % (sizeof(files_iter_rr) / sizeof(files_iter_rr[0]));
    rangeActive = false; // temp change for range sensor to not activate song
    keyActive = true;
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
}
