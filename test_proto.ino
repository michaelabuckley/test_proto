#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "board_wiring.h"
#include "adc.h"
#include "oled.h"



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

void draw_all_samples(Sample *r) {

 // double height.
  display.setTextSize(1,2);
// erase as we write text
  display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
  //display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);

  display.printf("A %4d  B %4d C %4d\n", r->sAA1, r->sBB1, r->sCC1);
  display.printf("AB%4d AC %4d BC%4d\n", r->sAB, r->sAC, r->sBC);

  display.display();
}

bool msg_active = false;
unsigned long msg_until  = 0;

void display_msg(const char *s) {
  draw_msg(s);
}

void draw_msg(const char *s) {
  display.setTextSize(2,2);
// erase as we write text
  display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
  //display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);

  display.println(s);

  display.display();
  delay(400);
}

#define DISPLAY_MODE_AVG 0
#define DISPLAY_MODE_MAX 1
#define DISPLAY_MODE_CALIBRATION 2

char display_mode = DISPLAY_MODE_AVG;

extern int32_t calib_floor;
extern int32_t calib_10_delta;

void draw_calibration() {

 // double height.
  display.setTextSize(1,2);
// erase as we write text
  display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
  //display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);

  display.printf("Floor.  : %4d     \n", calib_floor);
  display.printf("delta 10: %4d     \n", calib_10_delta);

  display.display();
}

void change_mode() {
  display_mode = (display_mode + 1) % 3;
  char s[30];
  sprintf(s, "Mode: %d", display_mode);
  display_msg(s);
}

void loop() {
  // buttons are pulled high, go low for click
  if (!digitalRead(BUTTON_A)) {
    display_msg("button A");
  }
  if (!digitalRead(BUTTON_B)) {
    external_calibration();
    display_msg("Update sample");
  }
  if (!digitalRead(BUTTON_C)) {
    change_mode();
  }
  //Serial.println("loop()");
  take_sample();

  switch(display_mode) {
    case DISPLAY_MODE_AVG:
      draw_all_samples(&avg_resistance);
      break;
    case DISPLAY_MODE_MAX:
      draw_all_samples(&max_resistance);
      break;
    case DISPLAY_MODE_CALIBRATION:
      draw_calibration();
      break;
  }

}


