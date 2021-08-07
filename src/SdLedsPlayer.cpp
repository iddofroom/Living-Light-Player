#include "SdLedsPlayer.h"

void SdLedsPlayer::setup() {
  leds.begin();    
}

void SdLedsPlayer::load_file(const char *file_name) {
  if (!sd.begin()) {
    sd.initErrorHalt("SdFatSdioEX begin() failed");
  }
  sd.chvol();
  if (!current_file.open(file_name, O_RDONLY)) {
    sd.errorHalt("open failed");
  }
  Serial.println("open success");  
}

bool SdLedsPlayer::is_file_playing() {
  return current_file.isOpen();
}

bool SdLedsPlayer::show_next_frame() {
  
  if(!is_file_playing()) {
    return false;
  }
  
  int bytes_read = current_file.read(frame_buf, bytes_per_frame);
  if (bytes_read < 0) {
    sd.errorHalt("read failed");
  }  
  if(bytes_read == 0) {
    current_file.close();  
    return false;
  }
  if(bytes_read < bytes_per_frame) {
    Serial.print("read frame with missing bytes.");
    return false;
  }
  for(int i=0; i< total_pixels; i++) {
    leds.setPixel(i, frame_buf[3*i], frame_buf[3*i+1], frame_buf[3*i+2]);
  }
  leds.show();
  return true;
}
