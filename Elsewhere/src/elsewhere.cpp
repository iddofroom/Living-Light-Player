#include "SdLedsPlayer.h"
#include "state.h"

#define LEDS_PER_STRIP 254
#define MAX_BRIGHTNESS 255
#define DEFAULT_BRIGHTNESS 50  // range is 0 (off) to 255 (max brightness)
#define RELAY_PIN 0
#define BUTTON_PIN 1
#define KEY_PIN 23
#define KEY_DEBOUNCE_TIME 50
#define STATE_DEBOUNCE_TIME 2
#define ERRORLED1 22
// #define ERRORLED2 23


int curr_file_i = 0;
// enum State back_states[] = {BACK0, BACK1, BACK2, BACK3, BACK4, BACK5, BACK6, BACK7, BACK8, BACK9, BACK10, BACK11};
const char *files_iter_rr[] = {"1", "2"}; // Make sure file list is not longer than state list
// Song tracking
enum State state, prevState = IDLE;
unsigned long currSongTime = 0, songStartTime = 0, procTime = 0;
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
unsigned long frame_timestamp;
uint8_t brightness = DEFAULT_BRIGHTNESS; 

// On/off button
uint8_t OnOffState = LOW;

// Key Switch
uint8_t keyState = HIGH;
uint8_t lastKeyState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = KEY_DEBOUNCE_TIME;
uint8_t reading;

// Monitoring vars
unsigned long lastMonitorTime = 0;
unsigned long MonitorDelay = 5000;

void setup() {
  Serial.begin(115200);
  // wait until serial port opens for native USB devices
  // while (! Serial) {
  //   delay(1);
  // }
  Serial.println("Serial Port Started.");
  sd_leds_player.setup();
  sd_leds_player.setBrightness(brightness);
  Serial.print("Brightness set to: "); Serial.print(brightness); Serial.println(" out of 255.");
  
  // Relay setup
  pinMode(RELAY_PIN, OUTPUT);
  Serial.print("Relay pin set to pin: "); Serial.println(RELAY_PIN);

  // On/Off button setup
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  Serial.print("On/Off button pin set to pin: "); Serial.println(BUTTON_PIN);

  // Key switch setup
  pinMode(KEY_PIN, INPUT_PULLUP);
  Serial.print("GPIO pin for key switch set to: "); Serial.println(KEY_PIN);

  // Error LEDs setup
  pinMode(ERRORLED1, OUTPUT);
  // pinMode(ERRORLED2, OUTPUT);
  digitalWrite(ERRORLED1, LOW);
  Serial.print("Error LEDs pins set to: "); Serial.print(ERRORLED1); Serial.println(" "); //Serial.println(ERRORLED2);

  // Teensies State setup
  stateInit();
}

void loop() {
  // unsigned long tic = millis();
  
  // On/Off button reading
  OnOffState = digitalRead(BUTTON_PIN);

  // Key switch reading with debounce
  reading = digitalRead(KEY_PIN);

  if (reading != lastKeyState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  // Logic for On state - if key: turn on relay and play file, second press (after 2 seconds) turns off relay and stops file
  if (OnOffState == HIGH) { // On state
    if ((millis() - lastDebounceTime) > debounceDelay) { // check last press was long enough ago
      if (reading != keyState) {
        keyState = reading;
        if (keyState == LOW) { // only trigger on one edge of the switch
          if(!sd_leds_player.is_file_playing()) { // if no file is playing then it's first press
            Serial.println("key switch triggered when On");
            state = KEY1;
            // Turn on relay
            digitalWrite(RELAY_PIN, HIGH);
            Serial.println("Relay on");
            // Load file
            sd_leds_player.load_file(files_iter_rr[state-1]); 
            frame_timestamp = sd_leds_player.load_next_frame();
            Serial.print("Loaded LEDS file "); Serial.println(files_iter_rr[state-1]);
          }
          else {
            Serial.println("key switch triggered second time when On");
            state = STOP;
            // Turn off relay
            digitalWrite(RELAY_PIN, LOW);
            Serial.println("Relay off");
            // Stop file
            sd_leds_player.stop_file();
          }
        }
      }
    } 
  } else { // Off state
    // Turn off relay as long as we're in Off state
    digitalWrite(RELAY_PIN, LOW);
    if ((millis() - lastDebounceTime) > debounceDelay) { // check last press was long enough ago
      if (reading != keyState) {
        keyState = reading;
        if (keyState == LOW) { // only trigger on one edge of the switch
          state = KEY2;
          // Load file
          sd_leds_player.load_file(files_iter_rr[state-1]);
          frame_timestamp = sd_leds_player.load_next_frame();
          Serial.print("Loaded LEDS file"); Serial.println(files_iter_rr[state-1]);
        }
      }
    } 
  }
  lastKeyState = reading;

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
  //   lastMonitorTime = millis();
  // }
  
  // Serial.println(millis() - tic);
}
