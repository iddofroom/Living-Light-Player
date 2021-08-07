# Living-Light-Player - Devolap By Amir Blum , Oren Bigler , Guy Levitsky
Use Xlight to run animation on Teensy

**Xlight  Setup:**
For Each 170 Led we adding a E131 sACN packet.
1) press the  "Add E1.31 Button"
2) Chose the unicast option
3) and writh the next IP address : 127.0.0.1
4) Add The number of univers you need (Number of LED in project / 170 LED )

**Xlight  Layout:**

1) you can add Background Image of the Instalation 
2) use the Toolbar of the shpes to add Led Strings
3) For each shepe chose the number of Nodes/String - as the number of LED in the string
4) Chose the start Channel - from the univers number , or from the end of the previse string
5) in the controller connection TAB - chose DMX protocol - PORT 1 , and the DMX channle according to the univers number

# ll-teensy-player
Teensy library that play led frames from SD card. Library was written and tested for teensy 3.5, with OctoWS2811 and builtin SD slot on the board.

# Installation
Download the library as zip. open Arduino IDE and go to Sketch -> Include Library -> Add .ZIP library...

# Usage
User should have micro SD card with compatible files, in the format specified below. Goto File -> Examples -> ll-teensy-player to see a few examples on how to use. To use the library, #include "SdLedsPlayer.h" in your code, then define an object of type SdLedsPlayer. The object constructor receives number of pixels in each of the 8 strings, and two buffers for led data which the user mush define and supply to the library.

# Main loop
The library let the user handle the main loop, so that extra logic can be done between every frame.

User should call the function void setup() once to initialize the object.

Then it should call the function void load_file(const char *file_name) to load a new file from SD for display. The loaded file format should contain:

The file does not contain any header. only rgb buffers.
N frames
Each frame contains (8 * leds_per_strip) pixels. it is the user's responsibility to check that the loaded file was generated with the correct amount of pixels per strip.
Each pixel should contain 3 bytes of data. the data will be sent to the LED modules in the order found in the file, so user should match the file's RGB ordering to the LED harrdware RGB order.
Then user can apply any required appl-logic, and call the function bool show_next_frame() to read next frame from the SD card and send it to the LEDs. Function return true if all goes well, and false in case of error (no file loaded \ end of file \ coruppted data).

To detect if a new file should be loaded, user can call bool is_file_playing(). If the function returns false, a new file should be loaded to the library.
# python

This is a python3 script, that listen for sACN packets, aggregate them into frames, and write the frames to a givin file.

# Install
You need to have python 3 installed on your computer.
the project uses pipenv for dependency management. you can install pipenv with `pip install pipenv`
To check that pipenv is intall and found in PATH, run `pipenv --version`.
Then run pipenv shell to enter the venv.

# Usage
`python ./capture.py --help` will show help about how to run the program.

You need to use config file in json format that describe which univere numbers are expected, how many pixels are hold, and to which stip id, and pixel in pixel within the strip they should be map.

The script will wait for a full frame (e.g. for all the universes in the config file to be received), and then write the frame to the `out_file` (as specify as command line argument).

The frame will always hold exactly `number_of_strings` * `pixels_per_string` pixels, with 3 channels each.

Channels are written to the file in the order they are found in the sACN packet. That means that if you want to change RGB order (according to the order of the physical LEDs), you need to configure the sending application appropriately.

If you don't need all strings, or all pixels in a string, just don't configure them in the `config.json` file, and they will not be updated (will just always contain `0`)

## Stopping
The program will capture sACN frames as soon as they are receive, and will keep capturing as long as it runs. To stop the program (which will stop writing frames to the file) use `Ctrl + Break` on keyboard (Note that on some keyboards, "Break" is labeled as "Pause").
You can also use the command line option -f to set the total number of frames which will be written (script will exit automatically once all these frames are captured).

## Config file
The program should be configured with a json file that maps universes (as sent on sACN packets from applications like xLights or Vixen), to the physical LEDs for display.
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
In the above example, `1\2\17` are the universe numbers. They should match the universes sent on the sACN packets. LED sequence softwares support configuration of this value.

`string_id` is a value in range [0, number_of_strings - 1], for Teensy 3.5, it should be a value in the range 0-7.

`pixel_in_string` is the pixel offset in the givin string, for which we should map the beginning of the LEDs colors in the universe.

`num_of_pixels` is the number of pixels that we want to copy from the sACN packet, for display. According to the sACN format, it should be <= 170.

`pixel_in_string + num_of_pixels` should be in the range of [0 - pixel_in_string - 1], meaning - we cannot copy more pixels then they are in the string.

The above example says: 
- take the first 3 pixels from universe 1, and copy them to string 0 starting at beginning of the string (`"pixel_in_string": 0`)
- take the the 170 pixels from universe 2, and copy them to string 7 starting at pixel 829. Notice that we begin in pixel 829, and copy 170 pixels, meaning last pixel will be 829+170=999. That would work only if `pixel_in_string` from command line arguments is set to 1000 or above, otherwise we would need to copy data to non-existing pixels.
- take a single pixel (the first one) from universe 17, and copy it to string 4 on pixel 20 (from string start).

