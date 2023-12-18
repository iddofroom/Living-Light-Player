#include "SdLedsPlayer.h"
#include "state.h"
#include "SD.h"

#define LEDS_PER_STRIP 300
#define MAX_BRIGHTNESS 255
#define DEFAULT_BRIGHTNESS 50  // range is 0 (off) to 255 (max brightness)
#define STATE_DEBOUNCE_TIME 2
#define ERRORLED1 23
// #define ERRORLED2 23


int curr_file_i = 0;
enum State back_states[] = {BACK0, BACK1, BACK2, BACK3, BACK4, BACK5, BACK6, BACK7, BACK8, BACK9, BACK10, BACK11, BACK12, BACK13, BACK14};
const char *files_iter_rr[] = {"cave1", "cave2", "cave3", "cave4", "cave5", "cave6", "cave7", "cave8", "cave9", "cave10"}; // Make sure file list is not longer than state list
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
  while (! sd_leds_player.setup()) {
    Serial.println("SD card setup failed, fix and reset to continue");
    delay(1000);
  }
  Serial.println("SD card started.");
  sd_leds_player.setBrightness(brightness);
  Serial.print("Brightness set to: "); Serial.print(brightness); Serial.println(" out of 255.");
  
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
  
  // Background LED file loading
  if(!sd_leds_player.is_file_playing()) {
    state = back_states[curr_file_i];
    Serial.print("No file is playing, loading new file, number: "); Serial.println(files_iter_rr[state-1]);
    sd_leds_player.load_file(files_iter_rr[state-1]); // minus 1 to translate state to filename because IDLE state is 0
    curr_file_i = (curr_file_i + 1) % (sizeof(files_iter_rr) / sizeof(files_iter_rr[0]));
    frame_timestamp = sd_leds_player.load_next_frame();
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
  //   lastMonitorTime = millis();
  // }
  
  // Serial.println(millis() - tic);
}
