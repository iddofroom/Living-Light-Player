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


#define LEDS_PER_STRIP 105
#define MAX_BRIGHTNESS 255
#define DEFAULT_BRIGHTNESS 255  // range is 0 (off) to 255 (max brightness)
#define STATE_DEBOUNCE_TIME 2
#define ERRORLED1 23
// #define ERRORLED2 23


int curr_file_i = 0;
enum State back_states[] = {BACK0, BACK1, BACK2, BACK3, BACK4, BACK5, BACK6, BACK7, BACK8, BACK9};
enum State rfid_states[] = {RFID_CHIP};
const char *files_iter_rr[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "chip"};
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
  
  // Error LEDs setup
  pinMode(ERRORLED1, OUTPUT);
  // pinMode(ERRORLED2, OUTPUT);
  digitalWrite(ERRORLED1, LOW);
  Serial.print("Error LEDs pins set to: "); Serial.print(ERRORLED1); Serial.println(" "); //Serial.println(ERRORLED2);

  // Teensies State setup
  stateInit();

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

  // RFID reading using interrupts
  if (rfidBooted) {
    if (bNewInt) { //new read interrupt
      Serial.println(F("RFID reader interrupt triggered. "));
      if (rfidReadNuid(rfid, p_nuidPICC, sizeof(nuidPICC))) {
        state = RFID_CHIP; // checkUidTable(nuidPICC);
        if (state == RFID_CHIP) rfidKivseeFlag = true;
        status = sd_leds_player.load_file(files_iter_rr[state-1]);
        if (!status) {
          Serial.println("file load from SD failed");
          delay(1000);
        }
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
