#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "board_wiring.h"
#include "adc.h"
#include "oled.h"

//char *button_display = " ";


// int count = 0;
// #define PIN_D5 (5u)
// int p_test = 5;
// float r_ref = 3570.0F;



// 6 samples we collect.  ADC samples are < 16 bits.  int32 is large enough to do sums
extern Sample display_resistance;

void pinSetup() {
  digitalWrite(PIN_TADrv, HIGH);
  digitalWrite(PIN_TBDrv, HIGH);
  digitalWrite(PIN_TCDrv, HIGH);
  pinMode(PIN_TADrv, INPUT);
  pinMode(PIN_TBDrv, INPUT);
  pinMode(PIN_TCDrv, INPUT);

  pinMode(PIN_TA1, INPUT);
  digitalWrite(PIN_TA1, LOW);
  pinMode(PIN_TB1, INPUT);
  digitalWrite(PIN_TB1, LOW);
  pinMode(PIN_TC1, INPUT);
  digitalWrite(PIN_TC1, LOW);
}

void setup() {
  //Serial.begin(9600);
  Serial.println("initDisplay");

  initDisplay();

  // setup buttons
  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);

  Serial.println("IO test");

  analogSetup();
  pinSetup();

  calibrateLevels();
}


void loop() {
  //Serial.println("loop()");
  take_sample();

  Sample *r = &display_resistance;

 // double height.
  display.setTextSize(1,2);
// erase as we write text
  display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
  //display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);

  display.printf("A %4d  B %4d C %4d\n", r->sAA1, r->sBB1, r->sCC1);
  display.printf("AB%4d AC %4d BC%4d\n", r->sAB, r->sAC, r->sBC);
  // display.printf("A %4d  B %4d C %4d\n", adc.AA1, adc.BB1, adc.CC1);
  // display.printf("AB%4d AC %4d BC%4d\n", adc.AB, adc.AC, adc.BC);

  display.display();

}


