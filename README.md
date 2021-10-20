# Living-Light-Player
xLights software LED animation shows played on Teensy with music synchronization

**xLight Installation:**
Download the latest version of xLights from https://xlights.org/

**xLights Controllers Setup:**
For each 170 LEDs add an Ethernet E1.31 controller (a dmx universe)
1) press the "Add Ethernet" button
2) Choose unicast option
3) Set IP address to 127.0.0.1 (localhost)
4) Add as many controller universes as needed (Number of LEDs in project / 170), give each controller its own universe number

**Xlight  Layout:**
1) Add a background image of the installation for convenience, not mandatory 
2) Add LED objects using the models toolbar located above the image
3) For each model shape choose the number of Nodes/String - the number of LEDs in the shape
4) Choose Start Channel - as Universe Number or End of Model of previous string
5) In Controller Connection - choose dmx protocol and port 1, set the dmx channel according to universe number of the string

For example xLights configuration load the xlights_networks.xml and xlights_rgbeffects.xml files from the xLights folder

# Python ll-sacn-recorder
Original python script project by Amir Blum can be found at https://github.com/blumamir/ll-sacn-recorder  
Forked version for CaveOfWonders project can be found at https://github.com/bigoren/ll-sacn-recorder  
The python3 script capture.py listens for sACN packets and aggregates their data into frames, writing them to a specified output file.  
The recorded file can be used as data for driving LEDs.

## Installation and Dependencies
Install python 3, the script requires the sacn package, you can either install it directly `pip install sacn` or use `pipenv`  
Install pipenv using `pip install pipenv`  
To check that pipenv is installed and found in PATH, run `pipenv --version`.  
Then run pipenv shell to enter the virtual env.

## Script Usage
`python ./capture.py --help` shows the help menu about how to run the script.

A configuration file in json format describing which universe numbers are expected, how many pixels each holds `num_of_pixels`, to which string id they are mapped and at what pixel within the string the universe starts `pixel_in_string` is required.

The script waits for a full frame, e.g. for all the universes in the configuration file to be received, and then writes the frame to the `out_file` as specified in the script command line argument.

A frame always holds exactly `number_of_strings` * `pixels_per_string` pixels * `3` (channels per pixel) data bytes.

Channels are written to the file in the order they are found in the sACN packet. That means that to change the RGB order to match the order of the physical LEDs the sending application should be configured appropriately.

If not all strings are in use or not all pixels in a string, they don't need to be configured in the `config.json` file, they will not be updated, output file contains 0's for strings not in use up to `number_of_strings` specified as command line argument.
Note that `number_of_strings` must match at teensy code set at 8 by default.

## Stopping
The script captures sACN packets as soon as they are received and keeps capturing as long as it is running or until reaching `frames_to_capture` as specified in the command line argument.  
To stop the script and writing of frames to the output file, use `Ctrl + Break` on the keyboard, note that on some keyboards "Break" is labeled as "Pause".  
When using command line option -f to set `frames_to_capture` the script automatically exits once the requested number of frames are captured.

## Config file
The script requires json configuration file that maps universes as sent on sACN packets from LED mapping applications, like xLights or Vixen, to the physical LEDs for display.
Example file should look like this:
```json
{
  "1": {
    "string_id": 0,
    "pixel_in_string": 0,
    "num_of_pixels": 3
  },
  "2": {
    "string_id": 7,
    "pixel_in_string": 829,
    "num_of_pixels": 170
  },
  "17": {
    "string_id": 4,
    "pixel_in_string": 20,
    "num_of_pixels": 1
  }
}
``` 
In the above example, `1\2\17` are the universe numbers. They should match the universes sent on the sACN packets. LED sequence software support configuration of this value.

`string_id` is a value in range [0, number_of_strings - 1], for Teensy 3.5, it should be a value in the range 0-7.

`pixel_in_string` is the pixel offset in the given string for which we should map the beginning of the LEDs colors of the universe.

`num_of_pixels` is the number of pixels that is copied from the sACN packet for display. According to the sACN format this should be <= 170.

`pixel_in_string + num_of_pixels` should be in the range of [0, pixels_per_string - 1], the script can't copy more pixels then there are in the string.

The above example says: 
- Take the first 3 pixels from universe 1 and copy them to string 0 starting at beginning of the string (`"pixel_in_string": 0`)
- Take the the 170 pixels from universe 2 and copy them to string 7 starting at pixel 829. Notice that the beginning is pixel 829 and copy size is 170 pixels, meaning last pixel will be 829+170=999. This works only if `pixel_in_string` from command line argument is set to 1000 or above, otherwise the script attempts to copy data from non-existing pixels.
- Take a single pixel (the first one) from universe 17, and copy it to string 4 on pixel 20 (from string start).

# Teensy
A Teensy board (version 3.5) with a builtin SD card and an OctoWS2811 shield plays the recorded python output file.  
The show music is played by another Teensy board using the audio library and synced to the LEDs playing Teensy.

## ll-teensy-player library
A Teensy library playing data frames from an SD card on LEDs.  
Original version by Amir Blum can be found at https://github.com/blumamir/ll-teensy-player  
Forked version for CaveOfWonders project can be found at https://github.com/bigoren/ll-teensy-player  
The library was written and tested for teensy 3.5, with OctoWS2811 shield and the builtin SD card slot on the Teensy board.  
Note that the library depends on the Teensy SdFat library versions 1.x.x.

## Installation
Arduino IDE: download the library as zip, open Arduino IDE and go to Sketch -> Include Library -> Add .ZIP library...  
VSCode with PlatformIO: The platformio.ini lib_deps field automatically fetches the required libraries, see platformio.ini under the CaveOfWonders folder.
Install SdFat @ 1.1.4
## Usage
Load a microSD card with library compatible files, these can be generated by the python capture.py script.  
To use the library, #include "SdLedsPlayer.h" in your code, then define an object of type SdLedsPlayer. 
The object constructor receives `pixels_per_string`, used for all 8 strings, and two buffers for led data which the user must define and supply to the library.  
Goto File -> Examples -> ll-teensy-player to see a few examples on library usage.

## Main loop
The library does not handle the main loop so that the user can add extra logic to be done between frames.

The `setup()` function should be called once to initialize the object.

Then the function `load_file(const char *file_name)` should be called to load a new file from SD for display. 

The loaded file format: 
* NO header, only rgb frames 
* N frames in a single file
* Each frame contains (8 * leds_per_strip) pixels. it is the user's responsibility to check that the loaded file was generated with the correct amount of pixels per strip.
* Each pixel should contain 3 bytes of data. the data is sent to the LED modules in the order found in the file, so user should match the file's RGB ordering to the LED hardware RGB order.

Any required application logic can be added but it is up to the user to keep frames timing and call the function `bool show_next_frame()` which will read the next frame from the SD card and send it to the LEDs. 
Function return true if all goes well and false in case of error (no file loaded \ end of file \ corrupted data).

The user can call `bool is_file_playing()` to check if a file is currently playing or not.

# CaveOfWonders
An art installation playing a xLights LEDs sequence on a Teensy 3.5 board using the ll-teensy-player library, a VL53L0X Time Of Flight sensor and key switch button.  
A second Teensy board is playing the sequence music and is synced to the first Teensy board using GPIOs.
