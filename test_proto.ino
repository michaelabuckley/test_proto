#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "board_wiring.h"
#include "adc.h"
#include "oled.h"
#include "sample.h"

void display_current_mode();
void draw_msg(const char *s);


#define DISPLAY_MODE_AVG 0
#define DISPLAY_MODE_MAX 1
#define DISPLAY_MODE_AVG_RAW 2
#define DISPLAY_MODE_MAX_RAW 3
#define DISPLAY_MODE_CALIBRATION 4
#define DISPLAY_MODE_COUNT 5

char display_mode = DISPLAY_MODE_AVG;


void pinSetup() {
  pinMode(PIN_TA, INPUT);
  pinMode(PIN_TB, INPUT);
  pinMode(PIN_TC, INPUT);
  pinMode(PIN_TADrv, INPUT);
  pinMode(PIN_TBDrv, INPUT);
  pinMode(PIN_TCDrv, INPUT);

  pinMode(PIN_TA1, INPUT);
  pinMode(PIN_TB1, INPUT);
  pinMode(PIN_TC1, INPUT);
  pinMode(PIN_TA1Drv, INPUT);
  pinMode(PIN_TB1Drv, INPUT);
  pinMode(PIN_TC1Drv, INPUT);
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

  //analogSetup();
  pinSetup();
  sampleSetup();

  calibrateLevels();

  display_current_mode();

}

char line0[] = "A 9999  B 9999 C 9999";
char line1[] = "AB9999 AC 9999 BC9999";
char buf[20] = "";

void format_fixed(char *out, int32_t val) {
  if (val<0) {
    sprintf(buf, " 0.0");
  } else if (val<500) {
    sprintf(buf, "%2d.%d", val/10, val%10);
  } else if (val > 9999) {
    sprintf(buf, "----");
  } else {
    sprintf(buf, "%4d", val);
  }
  memcpy(out, buf, 4);
}

void draw_all_samples_fixed(Sample *r) {

 // double height.
  display.setTextSize(1,2);
// erase as we write text
  display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
  //display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);

  format_fixed(line0+2, r->sAA1);
  format_fixed(line0+10, r->sBB1);
  format_fixed(line0+17, r->sCC1);
  format_fixed(line1+2, r->sAB);
  format_fixed(line1+10, r->sAC);
  format_fixed(line1+17, r->sBC);

  display.println(line0);
  display.println(line1);
//  display.printf("A %4d  B %4d C %4d\n", r->sAA1, r->sBB1, r->sCC1);
//  display.printf("AB%4d AC %4d BC%4d\n", r->sAB, r->sAC, r->sBC);

  display.display();
}

inline int32_t clamp(int32_t val) {
  if (val < 0) {
    return 0;
  } else if (val > 9999) {
    return 9999;
  } else {
    return val;
  }
}

void draw_all_samples(Sample *r) {

 // double height.
  display.setTextSize(1,2);
// erase as we write text
  display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
  //display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);

  display.printf("A %4d  B %4d C %4d\n", clamp(r->sAA1), clamp(r->sBB1), clamp(r->sCC1));
  display.printf("AB%4d AC %4d BC%4d\n", clamp(r->sAB), clamp(r->sAC), clamp(r->sBC));

  display.display();
}


bool msg_active = false;
unsigned long msg_until  = 0;

void display_msg(const char *s) {
  draw_msg(s);
}

void draw_msg(const char *s) {
  display.clearDisplay();

  display.setTextSize(2,2);
// erase as we write text
  display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
  //display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);

  display.println(s);

  display.display();
  delay(400);
}

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
  display_mode = (display_mode + 1) % DISPLAY_MODE_COUNT;
  display_current_mode();
}

void display_current_mode() {
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
      draw_all_samples_fixed(&resistance_avg);
      break;
    case DISPLAY_MODE_MAX:
      draw_all_samples_fixed(&resistance_max);
      break;
    case DISPLAY_MODE_AVG_RAW:
      draw_all_samples(&adc_avg);
      break;
    case DISPLAY_MODE_MAX_RAW:
      draw_all_samples(&adc_max);
      break;
    case DISPLAY_MODE_CALIBRATION:
      draw_calibration();
      break;
  }

}


