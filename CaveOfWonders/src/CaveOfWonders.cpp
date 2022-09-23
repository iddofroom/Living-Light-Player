#include "SdLedsPlayer.h"
#include "Adafruit_VL53L0X.h"
#include "state.h"
#include "rfid.h"

/*
--------------------------
|      RFID WIRING       |
--------------------------
|Teensy 4.1 ->    RFID   |
--------------------------
|    G      |      G     |
|    3.3V   |     3.3V   |
|    1      |     IRQ    |
|    9      |     RST    |
|    10     |     SDA    |
|    11     |     MOSI   |
|    12     |     MISO   |
|    13     |     SCK    |
--------------------------
*/

#define SS_PIN 10
#define RST_PIN 9
#define IRQ_PIN 1   // Configurable, depends on hardware


#define LEDS_PER_STRIP 254
#define MAX_BRIGHTNESS 255
#define DEFAULT_BRIGHTNESS 50  // range is 0 (off) to 255 (max brightness)
#define RANGE_BOOT_RETRIES 3
#define TOF_MEAS_INTERVAL 40
#define KEYPIN 22
#define KEY_DEBOUNCE_TIME 50
#define STATE_DEBOUNCE_TIME 2
#define ERRORLED1 23
// #define ERRORLED2 23


int curr_file_i = 0;
enum State back_states[] = {BACK0, BACK1, BACK2};
enum State rfid_states[] = {RFID_QUEEN, RFID_UNDER, RFID_COME};
const char *files_iter_rr[] = {"amir", "cave2", "cave1", "cave2", "come", "queen", "queen", "come", "come", "kivsee"};
// Song tracking
enum State state, prevState = IDLE;
unsigned long currSongTime = 0, songStartTime = 0, lastRangeTime = 0, procTime = 0;
unsigned long stateDebounceDelay = STATE_DEBOUNCE_TIME;


/*
 * SdLedsPlayer is the class that handles reading frames from file on SD card,
 * and writing it to the leds.
 * It needs to be initialized with LEDS_PER_STRIP (must match the leds per strips used in the generation
 * of the file written to SD card).
 * It also needs to receive the leds buffer for OctoWS2811, should be initialized as follows
 */
DMAMEM int display_memory[LEDS_PER_STRIP * 6]; 
int drawing_memory[LEDS_PER_STRIP * 6];
SdLedsPlayer sd_leds_player(LEDS_PER_STRIP, display_memory, drawing_memory);
bool status; // general use status variable
unsigned long frame_timestamp;
uint8_t brightness = DEFAULT_BRIGHTNESS; 

// Range sensor
Adafruit_VL53L0X lox = Adafruit_VL53L0X();
bool rangeBooted = false;
int rangeBootCnt = 0;
int range = -1;
int rangeAccum = -1;
// bool rangeActive = false; 
int rangeCnt = 0;

// Key Switch
int keyState = HIGH;
int lastKeyState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = KEY_DEBOUNCE_TIME;
// bool keyActive = true;
int reading;

// Monitoring vars
unsigned long lastMonitorTime = 0;
unsigned long MonitorDelay = 5000;

// RFID
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class
// Init array that will store new NUID 
byte nuidPICC[4] = {0x0, 0x0, 0x0, 0x0};
byte *p_nuidPICC = nuidPICC;
volatile bool bNewInt = false;
byte regVal = 0x7F;
bool rfidBooted = false;
bool rfidKivseeFlag = false;

/**
 * MFRC522 interrupt serving routine
 */
void readCard() {
  bNewInt = true;
}

void setup() {
  Serial.begin(115200);
  // wait until serial port opens for native USB devices
  // while (! Serial) {
  //   delay(1);
  // }
  Serial.println("Serial Port Started.");
  while (! sd_leds_player.setup()) {
    Serial.println("SD card setup failed, fix and reset to continue");
    delay(1000);
  }
  Serial.println("SD card started.");
  sd_leds_player.setBrightness(brightness);
  
  // Key switch setup
  pinMode(KEYPIN, INPUT_PULLUP);
  Serial.print("GPIO pin for key switch set to: "); Serial.println(KEYPIN);

  // Error LEDs setup
  pinMode(ERRORLED1, OUTPUT);
  // pinMode(ERRORLED2, OUTPUT);
  digitalWrite(ERRORLED1, LOW);
  Serial.print("Error LEDs pins set to: "); Serial.print(ERRORLED1); Serial.println(" "); //Serial.println(ERRORLED2);

  // Teensies State setup
  stateInit();

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
      digitalWrite(ERRORLED1, HIGH);
    } else {
      Serial.println(F("VL53L0X sensor started in continuous ranging mode.\n"));
    }
  } else {
    Serial.println(F("Failed to boot VL53L0X, continuing without range sensor, restart teensy to retry."));
    digitalWrite(ERRORLED1, HIGH);
  }

  // RFID reader setup
  rfidBooted = rfidInit(rfid);
  if (rfidBooted) {
    Serial.println(F("RFID Card reader initialized successfully."));
  } else {
    Serial.println(F("RFID initialization failed, continuing without it and turning on ERROR LED"));
    digitalWrite(ERRORLED1, HIGH);
  }
  // interrupt section
  /* setup IRQ pin */
  pinMode(IRQ_PIN, INPUT_PULLUP);
  /* Allow selected irq to be propagated to IRQ pin */
  regVal = 0xA0; // select rx irq
  rfid.PCD_WriteRegister(rfid.ComIEnReg, regVal);
  /* Activate interrupt in teensy */
  attachInterrupt(digitalPinToInterrupt(IRQ_PIN), readCard, FALLING);
  // set interrupt flag
  bNewInt = false;
  
  // Some delay to allow the audio teensy to wake up first
  delay(1000);
}

void loop() {
  // unsigned long tic = millis();
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
        if (range > (MAX_BRIGHTNESS - DEFAULT_BRIGHTNESS)) { 
          brightness = DEFAULT_BRIGHTNESS;
          sd_leds_player.setBrightness(brightness);
          // rangeActive = false; // disabling activation of song by range sensor by jumping from long distance
        }
        else if ((range <= (MAX_BRIGHTNESS - DEFAULT_BRIGHTNESS)) && range > 0) {
          brightness = MAX_BRIGHTNESS - range;
          sd_leds_player.setBrightness(brightness);
          // rangeActive = true; // enabling activation of song by range sensor from short distance
        }
        // else if ((range < 40) && rangeActive && !keyActive) { // don't allow range sensor song triggering from long distance or key switch triggered
        //   Serial.print("range sensor triggered at distance (mm): "); Serial.println(range);
        //   state = RANGE;
        //   sd_leds_player.load_file(files_iter_rr[state-1]);
        //   rangeActive = false;
        //   keyActive = true; // using the keyActive to make sure the range sensor can't trigger during its own song until next background song
        //   frame_timestamp = sd_leds_player.load_next_frame();
        // }
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
        Serial.println("key switch triggered! loading LEDs file");
        state = KEY;
        status = sd_leds_player.load_file(files_iter_rr[state-1]);
        if (!status) {
          Serial.println("file load from SD failed");
          delay(1000);
        }
        // keyActive = true;
        frame_timestamp = sd_leds_player.load_next_frame();
      }
    }
  }
  lastKeyState = reading;

  // RFID reading using interrupts
  if (rfidBooted) {
    if (bNewInt) { //new read interrupt
      Serial.println(F("RFID reader interrupt triggered. "));
      if (rfidReadNuid(rfid, p_nuidPICC, sizeof(nuidPICC))) {
        state = checkUidTable(nuidPICC);
        if (state == RFID_KIVSEE) rfidKivseeFlag = true;
        status = sd_leds_player.load_file(files_iter_rr[state-1]);
        if (!status) {
          Serial.println("file load from SD failed");
          delay(1000);
        }
        // keyActive = true; // using the keyActive to make sure the range sensor can't trigger until next background song
        frame_timestamp = sd_leds_player.load_next_frame();
        Serial.print(F("RFID card detection set state to: ")); Serial.println(state);
      }
      clearInt(rfid);
      rfid.PICC_HaltA(); // Halting the PICC takes relatively alot of time but it doesn't matter as we change the track anyways
      bNewInt = false;
      // Serial.println(millis() - tic);
    }
    activateRec(rfid);
  }

  // Background LED file loading
  if(!sd_leds_player.is_file_playing()) {
    if (rfidKivseeFlag) {
      uint8_t rand_num = random((uint8_t) 0, (uint8_t) (sizeof(rfid_states) / sizeof(rfid_states[0])));
      Serial.print("Kivsee flag detected, playing random RFID file number: "); Serial.println(rand_num);
      state = rfid_states[rand_num];
      status = sd_leds_player.load_file(files_iter_rr[state-1]); // minus 1 to translate state to filename because IDLE state is 0
      if (!status) {
        Serial.println("file load from SD failed");
        delay(1000);
      }
      // keyActive = true; // using the keyActive to make sure the range sensor can't trigger until next background song
      frame_timestamp = sd_leds_player.load_next_frame();
      rfidKivseeFlag = false;
    } 
    else {
      state = back_states[curr_file_i];
      Serial.print("No file is playing, loading new file number: "); Serial.println(files_iter_rr[state-1]);
      status = sd_leds_player.load_file(files_iter_rr[state-1]); // minus 1 to translate state to filename because IDLE state is 0
      if (!status) {
        Serial.println("file load from SD failed");
        delay(1000);
      }
      curr_file_i = (curr_file_i + 1) % (sizeof(back_states) / sizeof(back_states[0]));
      // keyActive = false;
      nuidPICC[0] = 0x0; nuidPICC[1] = 0x0; nuidPICC[2] = 0x0; nuidPICC[3] = 0x0;
      frame_timestamp = sd_leds_player.load_next_frame();
    }
  }

  // State tracking between two teensies
  if (state != prevState) {
    songStartTime = millis();
    stateEncode(state);
  }
  prevState = state;
  // Holding non IDLE state for a short while so we can use debounce on second teensy to capture state safely
  if (state != IDLE) {
    if ((millis() - songStartTime) > stateDebounceDelay) {
      state = IDLE;
      songStartTime = millis();
    }
  } 

  // Current song frame tracking
  currSongTime = millis() - songStartTime;
  if (currSongTime >= frame_timestamp) {
    sd_leds_player.show_next_frame();
    frame_timestamp = sd_leds_player.load_next_frame();
  }

  // Monitor printing, not really needed
  // if((millis() - lastMonitorTime) > MonitorDelay) {
  //   Serial.println(F("COW Leds Alive"));
  //   Serial.print(F("rangeActive: ")); Serial.println(rangeActive ? "true" : "false");
  //   Serial.print(F("keyActive: ")); Serial.println(keyActive ? "true" : "false");
  //   lastMonitorTime = millis();
  // }
  
  // Serial.println(millis() - tic);
}
