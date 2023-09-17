#include "SdLedsPlayer.h"
#include "state.h"
#include "SD.h"

// boot mode
#define BOOT_MODE_PIN 23

// leds
#define LEDS_PER_STRIP 254
#define MAX_BRIGHTNESS 255
#define DEFAULT_BRIGHTNESS 50 // range is 0 (off) to 255 (max brightness)

DMAMEM int display_memory[LEDS_PER_STRIP * 6];
int drawing_memory[LEDS_PER_STRIP * 6];
uint8_t brightness = DEFAULT_BRIGHTNESS;
unsigned long frame_timestamp;

// audio
#define STATE_DEBOUNCE_TIME 1

int curr_file_i = 0;
enum State back_states[] = {BACK0, BACK1, BACK2, BACK3, BACK4, BACK5};
const char *files_iter_rr[] = {"0", "1", "2", "3", "4", "5", "rfid", "sunrise", "sunrise_a", "sunrise_m", "sunset", "sunset_a", "sunset_m"};
/*
 * SdLedsPlayer is the class that handles reading frames from file on SD card,
 * and writing it to the leds.
 * It needs to be initialized with LEDS_PER_STRIP (must match the leds per strips used in the generation
 * of the file written to SD card).
 * It also needs to receive the leds buffer for OctoWS2811, should be initialized as follows
 */
SdLedsPlayer sd_leds_player(LEDS_PER_STRIP, display_memory, drawing_memory);

// Song tracking
enum State state, lastState, prevState = IDLE;
unsigned long currSongTime = 0, songStartTime = 0, lastRangeTime = 0, procTime = 0;
unsigned long stateDebounceDelay = STATE_DEBOUNCE_TIME;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = STATE_DEBOUNCE_TIME;

unsigned long lastPlayTime = 0;
unsigned long quietDelay = 600000; // in ms, divide by 60000 for minutes

// Monitoring vars
unsigned long lastMonitorTime = 0;
unsigned long MonitorDelay = 5000;

// playing mode
bool interactiveMode = true;
bool playingInteractiveSong = false;

// Logging
File loggerFile;

void initSdWriter()
{
    Serial.println("SD card started.");
    if (!SD.begin(BUILTIN_SDCARD))
    {
        Serial.println(F("SD card begin() failed"));
        return;
    }
    Serial.println(F("SD card begin() success"));
    loggerFile = SD.open("sunset.txt", FILE_WRITE);
}


void setup()
{
    Serial.begin(115200);
    // wait until serial port opens for native USB devices
    // while (! Serial) {
    //   delay(1);
    // }
    Serial.println("Serial Port Started.");

    while (!sd_leds_player.setup())
    {
        Serial.println("SD card setup failed, fix and reset to continue");
        delay(1000);
    }
    Serial.println("SD card started.");
    sd_leds_player.setBrightness(brightness);

    initSdWriter();
    loggerFile.println("Sunset Leds started");

    // boot mode
    pinMode(BOOT_MODE_PIN, INPUT_PULLUP);
    if (digitalRead(BOOT_MODE_PIN) == LOW)
    {
        interactiveMode = false;
        Serial.println("Boot mode pin is LOW, mode is set to AUTO");
        loggerFile.println("Boot mode pin is LOW, mode is set to AUTO");
    } else {
        interactiveMode = true;
        Serial.println("Boot mode pin is HIGH, mode is set to INTERACTIVE");
        loggerFile.println("Boot mode pin is HIGH, mode is set to INTERACTIVE");
    }
    loggerFile.flush();

    // Error LEDs setup
    //   pinMode(ERRORLED1, OUTPUT);
    //   digitalWrite(ERRORLED1, LOW);
    //   Serial.print("Error LEDs pins set to: ");
    //   Serial.print(ERRORLED1);
    //   Serial.println(" ");

    // Some delay to allow the audio teensy to wake up first
    delay(1000);
}

void loop()
{
    // Do not accept state changes if we are playing a song requested by user
    if (!playingInteractiveSong) {
        // Read state from GPIOs and decode
        State reading = stateDecode();
        if (reading != lastState) {
            // reset the debouncing timer
            lastDebounceTime = millis();
        }
        if ((millis() - lastDebounceTime) > debounceDelay) {
            if (reading != state) {
                prevState = state;
                state = reading;
                Serial.print("New state received: "); Serial.println(state);
                loggerFile.println("New state received: " + String(state) + " on GPIO interface");
                loggerFile.flush();
                sd_leds_player.load_file(files_iter_rr[state - 1]); // -1 to translate state to filename because IDLE state is 0 and must not be used
                lastPlayTime = millis();
                playingInteractiveSong = true;
            }
        }
        lastState = reading;
    }

    // Background LED file loading, if no file is playing for too long
    if (!sd_leds_player.is_file_playing())
    {
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
            bool status = sd_leds_player.load_file(files_iter_rr[state - 1]); // minus 1 to translate state to filename because IDLE state is 0
            if (!status) {
                // Serial.println("file load from SD failed");
                delay(1000);
            } else {
                lastPlayTime = millis();
                frame_timestamp = sd_leds_player.load_next_frame();
            }
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

    // Current song frame tracking
    currSongTime = millis() - songStartTime;
    if (currSongTime >= frame_timestamp)
    {
        sd_leds_player.show_next_frame();
        frame_timestamp = sd_leds_player.load_next_frame();
    }

    // Monitor printing, not really needed
    // if((millis() - lastMonitorTime) > MonitorDelay) {
    //   Serial.println(F("COW Leds Alive"));
    //   lastMonitorTime = millis();
    // }
    
}
