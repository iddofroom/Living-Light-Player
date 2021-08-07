#ifndef SD_LEDS_PLAYER_H__
#define SD_LEDS_PLAYER_H__

#include "SdFat.h"
#include "OctoWS2811.h"


class SdLedsPlayer 
{

public:
SdLedsPlayer(unsigned int leds_per_strip, void *display_memory, void *drawing_memory) :
  total_pixels(leds_per_strip * NUM_OF_STRIPS),
  bytes_per_frame(total_pixels * CHANNELS_PER_PIXEL),
  leds(leds_per_strip, display_memory, drawing_memory, WS2811_GRB | WS2811_800kHz)
{
  frame_buf = (uint8_t *)malloc(bytes_per_frame);
}

public:
  // call once for initialization
  void setup();
  // will read the next frame from the loaded file and send it to the leds.
  // return true on success and false in case no frame is available.
  bool show_next_frame();

public:
  // read a new file from sd card for led display
  void load_file(const char *file_name);
  // return true if there is a file loaded, and there are more frames ready for display
  bool is_file_playing();

private:
  // setup config
  static const int NUM_OF_STRIPS = 8;
  static const int CHANNELS_PER_PIXEL = 3; // DO NOT CHANGE!
  const int total_pixels;
  const int bytes_per_frame;

private:
  // OctoWS2811 stuff
  OctoWS2811 leds;

private:
  // SdFat stuff
  uint8_t *frame_buf;
  SdFatSdioEX sd;
  File current_file;
  
};


#endif // SD_LEDS_PLAYER_H__
