#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "oled.h"


void setup() {
  Serial.begin(9600);

  initDisplay();

  Serial.println("IO test");

  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);

  // text display tests
  display.setTextSize(1,2);
  // SSD1306_INVERSE
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("A 2.2   B 1.2   C 1.2");
  display.println("   AB 1.2   BC 1.2");
  // display.print("Connecting to SSID\n'adafruit':");
  // display.print("connected!");
  // display.println("IP: 10.0.1.23");
  // display.println("Sending val #0");
  display.setCursor(0,0);
  display.display(); // actually display all of the above
}

void loop() {
    display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);

  if(!digitalRead(BUTTON_A)) display.print("A");
  if(!digitalRead(BUTTON_B)) display.print("B");
  if(!digitalRead(BUTTON_C)) display.print("C");
  delay(10);
  yield();
  display.display();
}