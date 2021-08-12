#include "Adafruit_VL53L0X.h"
#include "SdLedsPlayer.h"

#define LEDS_PER_STRIP 11
#define LEDS_FRAME_TIME 25
#define FILE_TO_PLAY "out42"
#define TOF_MEAS_INTERVAL 200
#define KEYPIN 22
#define DEBOUNCE_TIME 50
#define RELAYPIN1 1
#define RELAYPIN2 23
#define OUTGPIO1 0
#define OUTGPIO2 17
#define OUTGPIO3 16 // TODO check what IOs are available

int curr_file_i = 0;
const char *files_iter_rr[] = {"out", "out"};

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
bool sd_leds_bool = false;
uint8_t brightness = 30; // range is 0 (off) to 255 (full brightness)

Adafruit_VL53L0X lox = Adafruit_VL53L0X();
int range = -1;
bool rangeActive = true;

int keyState = HIGH;
int lastKeyState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = DEBOUNCE_TIME;
bool keyActive = true;
int reading;

unsigned long currFrameTime = 0, lastShowTime = 0, lastRangeTime = 0, procTime = 0;

void stateEncode (uint8_t state) {
  // Serial.println(state);
  switch (state) {
    case 0:
      digitalWrite(OUTGPIO1, LOW);
      digitalWrite(OUTGPIO2, LOW);
      digitalWrite(OUTGPIO3, LOW);
      break;

    case 1:
      digitalWrite(OUTGPIO1, HIGH);
      digitalWrite(OUTGPIO2, LOW);
      digitalWrite(OUTGPIO3, LOW);
      break;

    case 2:
      digitalWrite(OUTGPIO1, LOW);
      digitalWrite(OUTGPIO2, HIGH);
      digitalWrite(OUTGPIO3, LOW);
      break;

    case 3:
      digitalWrite(OUTGPIO1, HIGH);
      digitalWrite(OUTGPIO2, HIGH);
      digitalWrite(OUTGPIO3, LOW);
      break;

    case 4:
      digitalWrite(OUTGPIO1, LOW);
      digitalWrite(OUTGPIO2, LOW);
      digitalWrite(OUTGPIO3, HIGH);
      break;

    case 5:
      digitalWrite(OUTGPIO1, HIGH);
      digitalWrite(OUTGPIO2, LOW);
      digitalWrite(OUTGPIO3, HIGH);
      break;

    case 6:
      digitalWrite(OUTGPIO1, LOW);
      digitalWrite(OUTGPIO2, HIGH);
      digitalWrite(OUTGPIO3, HIGH);
      break;

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
  digitalWrite(RELAYPIN1, HIGH); // default state is on
  pinMode(RELAYPIN2, OUTPUT);
  digitalWrite(RELAYPIN2, HIGH); // default state is on
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
  if (lox.isRangeComplete() & rangeActive) {
    range = lox.readRangeResult();
    // Serial.print(millis());Serial.print(" : Distance (mm): "); Serial.println(range);
    if (range < 40) {
      Serial.print("range sensor triggered at distance (mm): "); Serial.println(range);
      digitalWrite(RELAYPIN1, LOW); // shutting off RELAYS
      digitalWrite(RELAYPIN2, LOW); // shutting off RELAYS
      sd_leds_player.load_file(FILE_TO_PLAY);
      lastShowTime = millis();
      rangeActive = false;
      keyActive = false;
      stateEncode(3);
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
      // key switch turns off RELAYS
      if (keyState == HIGH) {
        Serial.println("key switch triggered! loading LEDS file");
        sd_leds_player.load_file(FILE_TO_PLAY);
        stateEncode(4);
        lastShowTime = millis();
        digitalWrite(RELAYPIN1, LOW);
        digitalWrite(RELAYPIN2, LOW);
        rangeActive = false;
        keyActive = false;
      }
    }
  }
  lastKeyState = reading;

  if(!sd_leds_player.is_file_playing()) {
    Serial.println("No file is playing, loading new file");
    sd_leds_player.load_file(files_iter_rr[curr_file_i]);
    stateEncode(curr_file_i+1);
    curr_file_i = (curr_file_i + 1) % (sizeof(files_iter_rr) / sizeof(files_iter_rr[0]));
    lastShowTime = millis();
    digitalWrite(RELAYPIN1, HIGH); // Turn RELAYS on when playing default file
    digitalWrite(RELAYPIN2, HIGH); // Turn RELAYS on when playing default file
    rangeActive = true;
    keyActive = true;
  }

  currFrameTime = millis() - lastShowTime;
  if (currFrameTime >= (LEDS_FRAME_TIME-procTime)) {
    procTime = 0;
    // if (currFrameTime != LEDS_FRAME_TIME) {
    //   Serial.println(currFrameTime);
    // }
    if (currFrameTime > LEDS_FRAME_TIME){
      procTime = (currFrameTime)%LEDS_FRAME_TIME;
    }
    lastShowTime = millis();
    sd_leds_bool = false;
    sd_leds_bool = sd_leds_player.show_next_frame();
    if (!sd_leds_bool) {
      Serial.print("Show frame failed, flag is: ");Serial.println(sd_leds_bool);
    }
  }
}
