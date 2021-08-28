/*  OctoWS2811 BasicTest.ino - Basic RGB LED Test
    http://www.pjrc.com/teensy/td_libs_OctoWS2811.html
    Copyright (c) 2013 Paul Stoffregen, PJRC.COM, LLC

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.

  Required Connections
  --------------------
    pin 2:  LED Strip #1    OctoWS2811 drives 8 LED Strips.
    pin 14: LED strip #2    All 8 are the same length.
    pin 7:  LED strip #3
    pin 8:  LED strip #4    A 100 ohm resistor should used
    pin 6:  LED strip #5    between each Teensy pin and the
    pin 20: LED strip #6    wire to the LED strip, to minimize
    pin 21: LED strip #7    high frequency ringining & noise.
    pin 5:  LED strip #8
    pin 15 & 16 - Connect together, but do not use
    pin 4 - Do not use
    pin 3 - Do not use as PWM.  Normal use is ok.

  This test is useful for checking if your LED strips work, and which
  color config (WS2811_RGB, WS2811_GRB, etc) they require.
*/

#include <OctoWS2811.h>

#define KEYPIN 22

const int ledsPerStrip = 300;
const int stringNum = 8;
const int numLeds = ledsPerStrip*stringNum;

DMAMEM int displayMemory[numLeds];
int drawingMemory[numLeds];

const int config = WS2811_GRB | WS2811_800kHz;

OctoWS2811 leds(ledsPerStrip, displayMemory, drawingMemory, config);
int location = 0;

void colorthem(int color , int to)
{
  int r= location;
  for (int i=r; i < r+to; i++) {
    leds.setPixel(i, color);
  }
  location+=to;
}

void colorWipe(int color, int wait)
{
  for (int i=location+50; i < leds.numPixels(); i++) {
    leds.setPixel(i, color);
    leds.show();
    delayMicroseconds(wait);
  }
}

void leds_clear(int ledsNum)
{
  for (int i=0; i<ledsNum; i++) {
    leds.setPixel(i,0);
  }
}
 
// #define RED    0xFF0000
// #define GREEN  0x00FF00
// #define BLUE   0x0000FF
// #define YELLOW 0xFFFF00
// #define PINK   0xFF1088
// #define ORANGE 0xE05800
// #define WHITE  0xFFFFFF

// Less intense...

#define RED    0x160000
#define GREEN  0x001600
#define BLUE   0x000016
#define YELLOW 0x101400
#define PINK   0x120009
#define ORANGE 0x100400
#define WHITE  0x101010

int keyState = HIGH;
int lastKeyState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;
bool keyActive = true;
int reading;
int objNum = 1;
const int maxObjNum = 27;

void setup() {
  Serial.begin(115200);
  
  // Key switch setup
  pinMode(KEYPIN, INPUT_PULLUP);
  Serial.print("GPIO pin for key switch set to: "); Serial.println(KEYPIN);

  leds.begin();
  leds.show();
}

void loop() {
  // Serial.println("ALIVE");
  reading = digitalRead(KEYPIN);
  if (reading != lastKeyState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != keyState) {
      keyState = reading;
      if (keyState == HIGH) {
        objNum++;
        if (objNum > maxObjNum) objNum = 1;
        Serial.print("key switch triggered, object number is now: "); Serial.println(objNum-1);
        leds_clear(numLeds);
      }
    }
  }
  lastKeyState = reading;
  
  // int microsec = 2000000 / leds.numPixels();  // change them all in 2 seconds

  
  // string 1
  location = 0;
  if (objNum == 1 ) colorthem(RED,6);    else location += 6;     // 1
  if (objNum == 2 ) colorthem(BLUE, 11); else location += 11;    // 2
  if (objNum == 3 ) colorthem(GREEN, 9); else location += 9;     // 3
  if (objNum == 4 ) colorthem(RED,7);    else location += 7;     // 4
  if (objNum == 5 ) colorthem(YELLOW,12);else location += 12;    // 5
  if (objNum == 6 ) colorthem(RED,9);    else location += 9;     // 6
  if (objNum == 7 ) colorthem(BLUE,9);   else location += 9;     // 7
  if (objNum == 8 ) colorthem(YELLOW,9); else location += 9;     // 8
  if (objNum == 9 ) colorthem(RED,11);   else location += 11;    // 9
  if (objNum == 10) colorthem(BLUE,6);   else location += 6;     // 10
  if (objNum == 11) colorthem(YELLOW,6); else location += 6;     // 11
  if (objNum == 12) colorthem(RED,10);   else location += 10;    // 12
  if (objNum == 13) colorthem(GREEN,2);  else location += 2;     // 13
  if (objNum == 14) colorthem(BLUE,10);  else location += 10;    // 14
  if (objNum == 15) colorthem(YELLOW,6); else location += 6;     // 15
  if (objNum == 16) colorthem(GREEN,2);  else location += 2;     // 16
  if (objNum == 17) colorthem(RED,2);    else location += 2;     // 17
   
  // string 2
  location=ledsPerStrip;
  if (objNum == 1 ) colorthem(RED,8);     else location += 8;     // 1
  if (objNum == 2 ) colorthem(BLUE, 12);  else location += 12;    // 2
  if (objNum == 3 ) colorthem(GREEN, 12); else location += 12;    // 3
  if (objNum == 4 ) colorthem(RED,12);    else location += 12;    // 4
  if (objNum == 5 ) colorthem(BLUE, 18);  else location += 18;    // 5
  if (objNum == 6 ) colorthem(GREEN, 12); else location += 12;    // 6
  if (objNum == 7 ) colorthem(BLUE, 2);   else location += 2;     // 7
  if (objNum == 8 ) colorthem(RED,12);    else location += 12;    // 8
  if (objNum == 9 ) colorthem(BLUE, 6);   else location += 6;     // 9
  if (objNum == 10) colorthem(GREEN, 10); else location += 10;    // 10
  if (objNum == 11) colorthem(RED,10);    else location += 10;    // 11
  if (objNum == 12) colorthem(GREEN, 8);  else location += 8;     // 12
  if (objNum == 13) colorthem(BLUE, 12);  else location += 12;    // 13
  if (objNum == 14) colorthem(RED,10);    else location += 10;    // 14
  if (objNum == 15) colorthem(GREEN, 10); else location += 10;    // 15
  if (objNum == 16) colorthem(BLUE, 12);  else location += 12;    // 16
  if (objNum == 17) colorthem(RED,12);    else location += 12;    // 17
  if (objNum == 18) colorthem(GREEN, 12); else location += 12;    // 18
  if (objNum == 19) colorthem(BLUE, 10);  else location += 10;    // 19
  if (objNum == 20) colorthem(RED,6);     else location += 6;     // 20
  if (objNum == 21) colorthem(GREEN, 6);  else location += 6;     // 21
  if (objNum == 22) colorthem(BLUE, 10);  else location += 10;    // 22
  if (objNum == 23) colorthem(GREEN, 11); else location += 11;    // 23
  if (objNum == 24) colorthem(BLUE, 12);  else location += 12;    // 24
  if (objNum == 25) colorthem(RED,7);     else location += 7;     // 25
 
 // string 3
  location = ledsPerStrip*2;
  if (objNum == 1 ) colorthem(GREEN, 12); else location += 12;    // 1
  if (objNum == 2 ) colorthem(BLUE, 12);  else location += 12;    // 2
  if (objNum == 3 ) colorthem(RED,12);    else location += 12;    // 3
  if (objNum == 4 ) colorthem(GREEN, 12); else location += 12;    // 4
  if (objNum == 5 ) colorthem(BLUE, 10);  else location += 10;    // 5
  if (objNum == 6 ) colorthem(RED,12);    else location += 12;    // 6
  if (objNum == 7 ) colorthem(GREEN, 12); else location += 12;    // 7
  if (objNum == 8 ) colorthem(BLUE, 12);  else location += 12;    // 8
  if (objNum == 9 ) colorthem(RED,2);     else location += 2;     // 9
  if (objNum == 10) colorthem(GREEN, 2);  else location += 2;     // 10
  if (objNum == 11) colorthem(BLUE, 12);  else location += 12;    // 11
  if (objNum == 12) colorthem(RED,2);     else location += 2;     // 12
  if (objNum == 13) colorthem(GREEN, 8);  else location += 8;     // 13
  if (objNum == 14) colorthem(RED,2);     else location += 2;     // 14
  if (objNum == 15) colorthem(GREEN, 2);  else location += 2;     // 15
  if (objNum == 16) colorthem(BLUE, 10);  else location += 10;    // 16
  if (objNum == 17) colorthem(GREEN, 6);  else location += 6;     // 17
  if (objNum == 18) colorthem(RED,10);    else location += 10;    // 18
  if (objNum == 19) colorthem(BLUE, 12);  else location += 12;    // 19
  if (objNum == 20) colorthem(GREEN, 10); else location += 10;    // 20
  if (objNum == 21) colorthem(RED,16);    else location += 16;    // 21
  
  // string 4
  location = ledsPerStrip*3; 
  if (objNum == 1 ) colorthem(RED,6);     else location += 6;     // 1
  if (objNum == 2 ) colorthem(BLUE, 12);  else location += 12;    // 2
  if (objNum == 3 ) colorthem(GREEN, 12); else location += 12;    // 3
  if (objNum == 4 ) colorthem(RED,10);    else location += 10;    // 4
  if (objNum == 5 ) colorthem(BLUE, 12);  else location += 12;    // 5
  if (objNum == 6 ) colorthem(GREEN, 12); else location += 12;    // 6
  if (objNum == 7 ) colorthem(RED,12);    else location += 12;    // 7
  if (objNum == 8 ) colorthem(BLUE, 12);  else location += 12;    // 8
  if (objNum == 9 ) colorthem(GREEN, 12); else location += 12;    // 9
  if (objNum == 10) colorthem(RED,8);     else location += 8;     // 10
  if (objNum == 11) colorthem(BLUE, 12);  else location += 12;    // 11
  if (objNum == 12) colorthem(GREEN, 12); else location += 12;    // 12
  if (objNum == 13) colorthem(RED,12);    else location += 12;    // 13
  if (objNum == 14) colorthem(BLUE, 10);  else location += 10;    // 14
  if (objNum == 15) colorthem(GREEN, 12); else location += 12;    // 15
  if (objNum == 16) colorthem(RED,7);     else location += 7;     // 16
  if (objNum == 17) colorthem(BLUE, 10);  else location += 10;    // 17
  if (objNum == 18) colorthem(GREEN, 8);  else location += 8;     // 18
  if (objNum == 19) colorthem(RED,2);     else location += 2;     // 19
  if (objNum == 20) colorthem(BLUE, 2);   else location += 2;     // 20
  if (objNum == 21) colorthem(GREEN, 2);  else location += 2;     // 21
  if (objNum == 22) colorthem(RED,2);     else location += 2;     // 22
  if (objNum == 23) colorthem(BLUE, 2);   else location += 2;     // 23
  if (objNum == 24) colorthem(GREEN, 2);  else location += 2;     // 24
  if (objNum == 25) colorthem(RED,2);     else location += 2;     // 25
  if (objNum == 26) colorthem(BLUE, 10);  else location += 10;    // 26
  if (objNum == 27) colorthem(GREEN, 12); else location += 12;    // 27
 
  // string 5
  location =ledsPerStrip*4; 
  if (objNum == 1 ) colorthem(RED,88);    else location += 88;    // 1
  if (objNum == 2 ) colorthem(BLUE, 72);  else location += 72;    // 2
  if (objNum == 3 ) colorthem(GREEN, 10); else location += 10;    // 3
  if (objNum == 4 ) colorthem(RED,5);     else location += 5;     // 4
  if (objNum == 5 ) colorthem(BLUE, 5);   else location += 5;     // 5
  if (objNum == 6 ) colorthem(GREEN, 12); else location += 12;    // 6
  if (objNum == 7 ) colorthem(RED,10);    else location += 10;    // 7
 
  // string 6
  location =ledsPerStrip*5; 
  if (objNum == 1 ) colorthem(RED,79);    else location += 79;    // 1
  if (objNum == 2 ) colorthem(BLUE, 86);  else location += 86;    // 2
  if (objNum == 3 ) colorthem(GREEN, 12); else location += 12;    // 3
  
  // string 7
  location =ledsPerStrip*6; 
  colorthem(RED,35); 
  
  // string 8 
  location = ledsPerStrip*7; 
  colorthem(RED,29); 
  
//  colorthem(RED,3);
  //colorthem(BLUE, 8);
//  colorthem(GREEN, 12);
  // colorWipe(ORANGE, microsec);
  // colorWipe(ORANGE, microsec);
  // colorWipe(BLUE, microsec);
  // colorWipe(YELLOW, microsec);
  // colorWipe(PINK, microsec);
  // colorWipe(ORANGE, microsec);
  // colorWipe(WHITE, microsec);
  leds.show();
  delay(20);
}
