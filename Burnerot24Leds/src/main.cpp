
 #include <Wire.h>
#include <Adafruit_VL53L0X.h>
#include "SdLedsPlayer.h"

#define LEDS_PER_STRIP 254

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

Adafruit_VL53L0X lox = Adafruit_VL53L0X();

// הגדרת פינים
const int buttonPin1 = 34;
const int buttonPin2 = 35;
const int buttonPin3 = 36;
const int buttonPin4 = 37;
const int lightSensorPin = 23;

// משתנים למעקב אחר זמן ודגימת החיישן
unsigned long previousMillis = 0;
const long interval = 500;  // כל 5 שניות

void setup() {
  
  while (! sd_leds_player.setup()) {
    Serial.println("SD card setup failed, fix and reset to continue");
    delay(1000);
  }
  Serial.println("SD card started.");
  sd_leds_player.setBrightness(128);

  // הגדרת פינים ככניסות
  pinMode(buttonPin1, INPUT_PULLUP);
  pinMode(buttonPin2, INPUT_PULLUP);
  pinMode(buttonPin3, INPUT_PULLUP);
  pinMode(buttonPin4, INPUT_PULLUP);
   Serial.begin(115200);
  
  // להפעיל את החיישן
  if (!lox.begin()) {
    Serial.println(F("Failed to boot VL53L0X"));
    while (1);
  }

  Serial.println(F("VL53L0X Ready"));
}

void loop() {
  
  VL53L0X_RangingMeasurementData_t measure;
  
  // בדיקה אם כפתור 1 נלחץ
  if (digitalRead(buttonPin1) == LOW) {
    Serial.println("Button 1 pressed!");
  }
  
  // בדיקה אם כפתור 2 נלחץ
  if (digitalRead(buttonPin2) == LOW) {
    Serial.println("Button 2 pressed!");
  }
  // בדיקה אם כפתור 3 נלחץ
  if (digitalRead(buttonPin3) == LOW) {
    Serial.println("Button 3 pressed!");
  }
  // בדיקה אם כפתור 4 נלחץ
  if (digitalRead(buttonPin4) == LOW) {
    Serial.println("Button 4 pressed!");
  }
 
  
  // לוג כל 1 שניות עם כמות האור
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    int lightLevel = analogRead(lightSensorPin);
    Serial.print("Light level: ");
    Serial.println(lightLevel);
    
  lox.rangingTest(&measure, false); // העבר false כדי לא להדפיס מידע נוסף

  if (measure.RangeStatus != 4) {  // אם המדידה תקינה
    Serial.print(F("Distance (mm): "));
    Serial.println(measure.RangeMilliMeter);
  } else {
    Serial.println(F("Out of range"));
  }
  

  }
  delay(100);
}
