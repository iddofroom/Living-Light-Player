#include "SdLedsPlayer.h"

#define LEDS_PER_STRIP 1000
#define FILE_TO_PLAY "out1"

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

void setup() {
  Serial.begin(115200);
  Serial.println("Started.");
  sd_leds_player.setup();
}

void loop() {
  
  if(!sd_leds_player.is_file_playing()) {
    sd_leds_player.load_file(FILE_TO_PLAY);    
  }

  sd_leds_player.show_next_frame();
}
