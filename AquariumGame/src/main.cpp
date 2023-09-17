#include "SdLedsPlayer.h"
#include "state.h"
#include "rfid.h"
#include "SD.h"

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

// rfid
#define SS_PIN 10
#define RST_PIN 9
#define IRQ_PIN 1 // Configurable, depends on hardware
// #define ERRORLED1 23

MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class
volatile bool rfidUnhandledInterrupt = false;
bool rfidBooted = false;

// leds
#define LEDS_PER_STRIP 10
#define MAX_BRIGHTNESS 255
#define DEFAULT_BRIGHTNESS 50 // range is 0 (off) to 255 (max brightness)

#define LIGHT_THRESHOLD 900

DMAMEM int display_memory[LEDS_PER_STRIP * 6];
int drawing_memory[LEDS_PER_STRIP * 6];
uint8_t brightness = DEFAULT_BRIGHTNESS;
unsigned long frame_timestamp;

// buttons

#define BTN_DEBOUNCE_TIME 50
#define BTN_COUNT 10
#define MAX_IO 50 //50 is the max number of IO pins

int lastBtnsState[MAX_IO]; 
int btnsState[MAX_IO];
unsigned long lastDebouncedTime[MAX_IO];

unsigned long debounceDelay = BTN_DEBOUNCE_TIME;

#define BTN_BIG_RED 38

#define BTN_YELLOW 33
#define BTN_BLUE 34
#define BTN_RED 35
#define BTN_GREEN 36

#define BTN_ON_ON 37
#define KNOB 39
#define RELAY 40
#define LIGHT_SENSOR 41

#define JOYSTICK_UP 0
#define JOYSTICK_DOWN 3
#define JOYSTICK_LEFT 4
#define JOYSTICK_RIGHT 13

int knob_last_value = 0;

int buttons[] = { BTN_RED, BTN_YELLOW,
BTN_BLUE, BTN_GREEN, JOYSTICK_UP, JOYSTICK_DOWN,
JOYSTICK_LEFT, JOYSTICK_RIGHT, BTN_ON_ON };

File questLoggerFile;

int on_on_btn_state = LOW;

// audio
#define STATE_DEBOUNCE_TIME 2

int KNOB_CHANGE = 5; // minimum change to consider triggered
int curr_file_i = 0;
// Hidden coupling between files_iter_rr to this array
// enum State {IDLE, BACK0, RFID, COIN, ON_ON, STAGE1, STAGE2, STAGE3, STAGE4};
enum State back_states[] = { BACK0 };
const char *files_iter_rr[] = {"loop", "rfid", "coin", "on_on", "stage1", "stage2", "stage3", "stage4"};
/*
 * SdLedsPlayer is the class that handles reading frames from file on SD card,
 * and writing it to the leds.
 * It needs to be initialized with LEDS_PER_STRIP (must match the leds per strips used in the generation
 * of the file written to SD card).
 * It also needs to receive the leds buffer for OctoWS2811, should be initialized as follows
 */
SdLedsPlayer sd_leds_player(LEDS_PER_STRIP, display_memory, drawing_memory);

// Song tracking
enum State state, prevState = IDLE;
unsigned long currSongTime = 0, songStartTime = 0, lastRangeTime = 0, procTime = 0;
unsigned long stateDebounceDelay = STATE_DEBOUNCE_TIME;

// Monitoring vars
unsigned long lastMonitorTime = 0;
unsigned long MonitorDelay = 5000;

// RFID

/**
 * MFRC522 interrupt serving routine
 */
void readCard()
{
  // we only flag that we got interrupt.
  // this will be handled in the loop
  rfidUnhandledInterrupt = true;
}

void initSdWriter()
{
    Serial.println("SD card started.");
    if (!SD.begin(BUILTIN_SDCARD))
    {
        Serial.println(F("SD card begin() failed"));
        return;
    }
    Serial.println(F("SD card begin() success"));
    questLoggerFile = SD.open("aquarium.txt", FILE_WRITE);
}

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
  for (int i = 0; i < BTN_COUNT; i++) {
    pinMode(buttons[i], INPUT_PULLUP);   
  }
  
  pinMode(RELAY, OUTPUT);
  digitalWrite(RELAY, LOW);
  
  // we are initializing an array of maximum number of ports
  for (int i = 0; i < MAX_IO; i++) {
    lastBtnsState[i] = HIGH;
    btnsState[i] = HIGH;
    lastDebouncedTime[i] = 0;
  }
}

void setup()
{
  Serial.begin(115200);
  // wait until serial port opens for native USB devices
  // while (! Serial) {
  //   delay(1);
  // }
  Serial.println("Serial Port Started.");
  setup_buttons();
  Serial.println("Initialized Buttons.");
  
  while (!sd_leds_player.setup())
  {
    Serial.println("SD card setup failed, fix and reset to continue");
    delay(1000);
  }
  initSdWriter();
  sd_leds_player.setBrightness(brightness);

  // Teensies State setup
  stateInit();

  // RFID reader setup
  rfidBooted = rfidInit(rfid);
  if (rfidBooted)
  {
    Serial.println(F("RFID Card reader initialized successfully."));
  }
  else
  {
    Serial.println(F("RFID initialization failed, continuing without it and turning on ERROR LED"));
  }
  // interrupt section
  /* setup IRQ pin */
  pinMode(IRQ_PIN, INPUT_PULLUP);
  /* Allow selected irq to be propagated to IRQ pin */
  allowRfidRxInt(rfid);
  /* Activate interrupt in teensy */
  attachInterrupt(digitalPinToInterrupt(IRQ_PIN), readCard, FALLING);
  // set interrupt flag
  rfidUnhandledInterrupt = false;

  // // Some delay to allow the audio teensy to wake up first
  delay(1000);
}

void set_song_by_state() {
  bool status = sd_leds_player.load_file(files_iter_rr[state-1]); // -1 to translate state to filename because IDLE state is 0 and must not be used
    if (!status)
    {
      Serial.println("file load from SD failed");
      delay(1000);
    }
    frame_timestamp = sd_leds_player.load_next_frame();
}

void read_rfid_using_interrupts() {
  if (!rfidBooted) {
    return;
  }
  if (rfidUnhandledInterrupt)
  { // new read interrupt
    Serial.println(F("RFID reader interrupt triggered. "));
    state = RFID;
    set_song_by_state();
    
    Serial.print(F("Quest logic set state to: "));
    Serial.println(state);
    clearRfidInt(rfid);
    rfid.PICC_HaltA(); // Halting the PICC takes relatively alot of time but it doesn't matter as we change the track anyways
    rfidUnhandledInterrupt = false;
    // Serial.println(millis() - tic);
  }
  activateRfidReception(rfid);
}

// Knob is triggered if change from last read is bigger than KNOB_CHANGE
bool is_knob_triggered() {
  int knobValue = analogRead(KNOB);
  int absoluteChange = abs(knobValue - knob_last_value);
  if(absoluteChange > KNOB_CHANGE) {
    return true;
  }
  return false;
}

// Valid upon starting the game (aka big red button pressed)
void check_game_state() {
  if (state == STAGE1) {
    if(is_button_pressed(BTN_GREEN)) {
      state = STAGE2;
      set_song_by_state();
    }
  } else if (state == STAGE2 || state == STAGE3) {
    bool is_joystick_pressed = (
      is_button_pressed(JOYSTICK_UP) || 
      is_button_pressed(JOYSTICK_DOWN) ||
      is_button_pressed(JOYSTICK_LEFT) ||
      is_button_pressed(JOYSTICK_RIGHT)
      );

    if (is_joystick_pressed) {
      state = STAGE3; // LOUGH as long as people play with it
      set_song_by_state();
    }
    if(state == STAGE3 && is_knob_triggered()) {
      state = STAGE4;
      set_song_by_state();
    }
  }
}

bool is_light_sensor_triggered() {
  int lightValue = analogRead(LIGHT_SENSOR);
  // TODO - add debounced
  if (lightValue > LIGHT_THRESHOLD) {
    return true;
  }
  return false;
}

void loop()
{
  // Only on background state we should listen for events
  bool should_listen_for_events = state == BACK0;
  if(should_listen_for_events) {
    is_button_pressed(BTN_RED);
    is_button_pressed(BTN_BLUE);
    is_button_pressed(BTN_YELLOW);
    is_button_pressed(JOYSTICK_DOWN);
    is_button_pressed(JOYSTICK_UP);
    is_button_pressed(JOYSTICK_LEFT);
    is_button_pressed(JOYSTICK_RIGHT);

    if(is_button_pressed(BTN_ON_ON)) {
        state = ON_ON;
        set_song_by_state();
    }

    if(is_button_pressed(BTN_BIG_RED)) {
      // Start Game
      state = STAGE1;
      digitalWrite(RELAY, HIGH);
      set_song_by_state();
    }

    check_game_state();

    bool light_triggered = is_light_sensor_triggered();
    if(light_triggered) {
        state = COIN;
        set_song_by_state();
    }
    
    read_rfid_using_interrupts();
  }

  // If file ended - restart same file
  if (!sd_leds_player.is_file_playing())
  {
    state = BACK0; // Return to first background idle music
    Serial.print("No file is playing, restarting first file ");
    Serial.println(files_iter_rr[state-1]);
    set_song_by_state();
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

  // Current song frame tracking
  currSongTime = millis() - songStartTime;
  if (currSongTime >= frame_timestamp)
  {
    sd_leds_player.show_next_frame();
    frame_timestamp = sd_leds_player.load_next_frame();
  }

}
