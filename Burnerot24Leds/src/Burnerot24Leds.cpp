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
bool rfidEnabled = false;

// leds
#define LEDS_PER_STRIP 302  // CHANGE ACCORDING TO PROJECT LONGEST STRIP + 1 TO MATCH LED FILE
#define MAX_BRIGHTNESS 255
#define DEFAULT_BRIGHTNESS 50 // range is 0 (off) to 255 (max brightness)

DMAMEM int display_memory[LEDS_PER_STRIP * 6];
int drawing_memory[LEDS_PER_STRIP * 6];
uint8_t brightness = DEFAULT_BRIGHTNESS;
unsigned long frame_timestamp;

// Light sensor
#define LIGHT_PIN 35
#define LIGHT_SENSE_THR 900
int lightLevel;
int prevLightLevel;
bool lightSenseEnabled = false;

// GPIO
#define ARD_INPUT_PIN 34
#define SWITCH_MODE_PIN 36

// audio
#define STATE_DEBOUNCE_TIME 2

int curr_file_i = 0;
enum State back_states[] = {BACK0, BACK1, BACK2};
const char *files_iter_rr[] = {"kayla", "pachamama", "overthinker", "3", "4", "5", "pachamama", "overthinker"};
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

void resetController() {
  SCB_AIRCR = 0x05FA0004; // Request a system reset
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
    initSdWriter();
    sd_leds_player.setBrightness(brightness);

    // Error LEDs setup
    //   pinMode(ERRORLED1, OUTPUT);
    //   digitalWrite(ERRORLED1, LOW);
    //   Serial.print("Error LEDs pins set to: ");
    //   Serial.print(ERRORLED1);
    //   Serial.println(" ");

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
        // digitalWrite(ERRORLED1, HIGH);
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

    // Light sensor setup
    pinMode(LIGHT_PIN, INPUT);
    lightLevel = analogRead(LIGHT_PIN);
    prevLightLevel = lightLevel;

    // GPIO setup
    pinMode(ARD_INPUT_PIN, INPUT);
    pinMode(SWITCH_MODE_PIN, INPUT);

    // Some delay to allow the audio teensy to wake up first
    delay(1000);

    // Load first background file
    state = BACK0;
    bool status = sd_leds_player.load_file(files_iter_rr[state - 1]);
    if (!status)
    {
        Serial.println("file load from SD failed");
        delay(1000);
    }
    frame_timestamp = sd_leds_player.load_next_frame();

    // Enable RFID and light sensor by default according to switch position
    if (digitalRead(SWITCH_MODE_PIN) == LOW)
    {
        rfidEnabled = true;
        lightSenseEnabled = true;
    }
}

void loop()
{
    // unsigned long tic = millis();

    // RFID reading using interrupts
    if (rfidBooted && rfidEnabled)
    {
        if (rfidUnhandledInterrupt)
        { // new read interrupt
            Serial.println(F("RFID reader interrupt triggered. "));
            QuestState questState = handleQuestLogic(rfid);
            switch (questState)
            {
            case QUEST_STATE_DONE:
                state = RFID_DONE;
                break;
            default:
                break;
            }
            // Load leds file according to logic selected state
            if (state != IDLE)
            {
                rfidEnabled = false; // disable rfid reading when playing rfid song
                lightSenseEnabled = false; // disable light sensor when playing rfid song
                bool status = sd_leds_player.load_file(files_iter_rr[state - 1]);
                if (!status)
                {
                    Serial.println("file load from SD failed");
                    delay(1000);
                }
                frame_timestamp = sd_leds_player.load_next_frame();
            }
            Serial.print(F("Quest logic set state to: "));
            Serial.println(state);
            clearRfidInt(rfid);
            rfid.PICC_HaltA(); // Halting the PICC takes relatively alot of time but it doesn't matter as we change the track anyways
            rfidUnhandledInterrupt = false;
            // Serial.println(millis() - tic);
        }
        activateRfidReception(rfid);
    }

    // Arduino input used as RFID trigger
    if (digitalRead(ARD_INPUT_PIN) == LOW && rfidEnabled)
    {
        Serial.println(F("Arduino input triggered."));
        state = RFID_DONE;
        // Load leds file
        bool status = sd_leds_player.load_file(files_iter_rr[state - 1]);
        if (!status)
        {
            Serial.println("file load from SD failed");
            delay(1000);
        }
        frame_timestamp = sd_leds_player.load_next_frame();
        rfidEnabled = false; // disable rfid reading when playing rfid song
        lightSenseEnabled = false; // disable light sensor when playing rfid song
    }

    // Light sensor reading
    prevLightLevel = lightLevel;
    lightLevel = analogRead(LIGHT_PIN);
    if (lightSenseEnabled)
    {
        if ((lightLevel > LIGHT_SENSE_THR && prevLightLevel <= LIGHT_SENSE_THR) || (lightLevel <= LIGHT_SENSE_THR && prevLightLevel > LIGHT_SENSE_THR))
        {
            Serial.print("Light sensor triggered with level: "); Serial.print(lightLevel);
            Serial.print(" and previous light level: "); Serial.println(prevLightLevel);
            state = LIGHT_TRIGGERED;
            lightSenseEnabled = false; // disable light sensor when playing light song
            rfidEnabled = false; // disable rfid reading when playing light song
            // Load leds file
            bool status = sd_leds_player.load_file(files_iter_rr[state - 1]);
            if (!status)
            {
                Serial.println("file load from SD failed");
                delay(1000);
            }
            frame_timestamp = sd_leds_player.load_next_frame();
        }
    }

    // Background LED file loading
    if (!sd_leds_player.is_file_playing())
    {
        if (SWITCH_MODE_PIN == LOW) // sensors enabled mode, reset after every song
        {
            resetController();
        }
        else    //  sensors disabled mode, play background songs in a loop
        {
            state = back_states[curr_file_i];
            Serial.print("No file is playing, loading new file number: ");
            Serial.println(files_iter_rr[state - 1]);
            bool status = sd_leds_player.load_file(files_iter_rr[state - 1]); // minus 1 to translate state to filename because IDLE state is 0
            if (!status)
            {
                // Serial.println("file load from SD failed");
                delay(1000);
            }
            curr_file_i = (curr_file_i + 1) % (sizeof(back_states) / sizeof(back_states[0]));
            rfidEnabled = false;
            lightSenseEnabled = false;
            clearQuestCurrUid();
            frame_timestamp = sd_leds_player.load_next_frame();
        }
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
    //   Serial.println(F("Leds Alive"));
    //   lastMonitorTime = millis();
    // }

    // Serial.println(millis() - tic);
}
