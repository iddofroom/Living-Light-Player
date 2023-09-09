#include "SdLedsPlayer.h"
#include "state.h"
#include "rfid.h"
#include "quest.h"

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
#define LEDS_PER_STRIP 61
#define MAX_BRIGHTNESS 255
#define DEFAULT_BRIGHTNESS 50 // range is 0 (off) to 255 (max brightness)

DMAMEM int display_memory[LEDS_PER_STRIP * 6];
int drawing_memory[LEDS_PER_STRIP * 6];
uint8_t brightness = DEFAULT_BRIGHTNESS;
unsigned long frame_timestamp;

// buttons

#define BTN_DEBOUNCE_TIME 50
#define STATE_DEBOUNCE_TIME 2
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

#define BTN_EXTRA_1 37
#define BTN_EXTRA_2 39
#define RELAY 40
#define LIGHT_SENSOR 41

#define JOYSTICK_UP 0
#define JOYSTICK_DOWN 3
#define JOYSTICK_LEFT 4
#define JOYSTICK_RIGHT 13

int buttons[] = { BTN_RED, BTN_YELLOW,
BTN_BLUE, BTN_GREEN, JOYSTICK_UP, JOYSTICK_DOWN,
JOYSTICK_LEFT, JOYSTICK_RIGHT, BTN_EXTRA_1, BTN_EXTRA_2 };


// audio
#define STATE_DEBOUNCE_TIME 2

int curr_file_i = 0;
enum State back_states[] = {BACK0, BACK1, BACK2, BACK3, BACK4, BACK5};
const char *files_iter_rr[] = {"0", "1", "2", "3", "4", "5", "quest_fail", "quest_success", "quest_too_soon", "quest_done"};
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



  // // Key switch reading with debounce
  // int reading = digitalRead(btn_port);
  // if (reading != lastBtnsState[btn_port]) {
  //   // Reset the debouncing timer
  //   //lastDebouncedTime[btn_port] = millis();
  //   lastDTime = millis();
  // }
  // lastBtnsState[btn_port] = reading;
  // if ((millis() - lastDTime) > debounceDelay) {
  //   if (reading != lastBtnsState[btn_port]) {
  //     lastBtnsState[btn_port] = reading;
  //     if (lastBtnsState[btn_port] == HIGH) {
  //       Serial.println("Button triggered!");
  //       // Serial.println(btn_port);
  //       return true;
  //     }
  //   }
  // }
  // return false;
}

void setup_buttons() {
  for (int i = 0; i < BTN_COUNT; i++) {
    pinMode(buttons[i], INPUT_PULLUP);   
  }
  
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
  
  // while (!sd_leds_player.setup())
  // {
  //   Serial.println("SD card setup failed, fix and reset to continue");
  //   delay(1000);
  // }
  // Serial.println("SD card started.");
  // initSdWriter();
  // sd_leds_player.setBrightness(brightness);

  // Teensies State setup
  //stateInit();

  // RFID reader setup
  // rfidBooted = rfidInit(rfid);
  // if (rfidBooted)
  // {
  //   Serial.println(F("RFID Card reader initialized successfully."));
  // }
  // else
  // {
  //   Serial.println(F("RFID initialization failed, continuing without it and turning on ERROR LED"));
  // }
  // interrupt section
  /* setup IRQ pin */
  // pinMode(IRQ_PIN, INPUT_PULLUP);
  // /* Allow selected irq to be propagated to IRQ pin */
  // allowRfidRxInt(rfid);
  // /* Activate interrupt in teensy */
  // attachInterrupt(digitalPinToInterrupt(IRQ_PIN), readCard, FALLING);
  // // set interrupt flag
  // rfidUnhandledInterrupt = false;

  // // Some delay to allow the audio teensy to wake up first
  delay(1000);
}

void read_rfid_using_interrupts() {
  if (!rfidBooted) {
    return;
  }
  if (rfidUnhandledInterrupt)
  { // new read interrupt
    // Serial.println(F("RFID reader interrupt triggered. "));
    QuestState questState = handleQuestLogic(rfid);
    switch(questState) {
      case QUEST_STATE_FAILED:
        state = RFID_FAILED;
        break;
      case QUEST_STATE_SUCCESSFUL:
        state = RFID_SUCCESSFUL;
        break;
      case QUEST_STATE_TOO_SOON:
        state = RFID_TOO_SOON;
        break;
      case QUEST_STATE_DONE:
        state = RFID_DONE;
        break;
      default:
        break;
    }
    // Load leds file according to logic selected state
    // if (state != IDLE) 
    // {
    //   bool status = sd_leds_player.load_file(files_iter_rr[state - 1]);
    //   if (!status)
    //   {
    //     Serial.println("file load from SD failed");
    //     delay(1000);
    //   }
    //   frame_timestamp = sd_leds_player.load_next_frame();
    // }
    Serial.print(F("Quest logic set state to: "));
    Serial.println(state);
    clearRfidInt(rfid);
    rfid.PICC_HaltA(); // Halting the PICC takes relatively alot of time but it doesn't matter as we change the track anyways
    rfidUnhandledInterrupt = false;
    // Serial.println(millis() - tic);
  }
  activateRfidReception(rfid);
}

void loop()
{
  is_button_pressed(BTN_BIG_RED);
  is_button_pressed(BTN_RED);
  is_button_pressed(BTN_BLUE);
  is_button_pressed(BTN_GREEN);  
  is_button_pressed(BTN_YELLOW);
  is_button_pressed(JOYSTICK_DOWN);
  is_button_pressed(JOYSTICK_UP);
  is_button_pressed(JOYSTICK_LEFT);
  is_button_pressed(JOYSTICK_RIGHT);
  is_button_pressed(BTN_EXTRA_1);
  is_button_pressed(BTN_EXTRA_2);
  
  read_rfid_using_interrupts();

  // Background LED file loading

  // if (!sd_leds_player.is_file_playing())
  // {
  //   state = back_states[curr_file_i];
  //   Serial.print("No file is playing, loading new file number: ");
  //   Serial.println(files_iter_rr[state - 1]);
  //   bool status = sd_leds_player.load_file(files_iter_rr[state - 1]); // minus 1 to translate state to filename because IDLE state is 0
  //   if (!status)
  //   {
  //     // Serial.println("file load from SD failed");
  //     delay(1000);
  //   }
  //   curr_file_i = (curr_file_i + 1) % (sizeof(back_states) / sizeof(back_states[0]));
  //   clearQuestCurrUid();
  //   frame_timestamp = sd_leds_player.load_next_frame();
  // }

  // State tracking between two teensies
  // if (state != prevState)
  // {
  //   songStartTime = millis();
  //   stateEncode(state);
  // }
  // prevState = state;
  // // Holding non IDLE state for a short while so we can use debounce on second teensy to capture state safely
  // if (state != IDLE)
  // {
  //   if ((millis() - songStartTime) > stateDebounceDelay)
  //   {
  //     state = IDLE;
  //   }
  // }

  // Current song frame tracking
  // currSongTime = millis() - songStartTime;
  // if (currSongTime >= frame_timestamp)
  // {
  //   sd_leds_player.show_next_frame();
  //   frame_timestamp = sd_leds_player.load_next_frame();
  // }

}
