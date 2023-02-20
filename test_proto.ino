#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "adc.h"
#include "oled.h"
#include "board_wiring.h"

//char *button_display = " ";


int count = 0;
#define PIN_D5 (5u)
int p_test = 5;
float r_ref = 3570.0F;

uint32_t adc_AA1;
uint32_t adc_AB;
uint32_t adc_AC;
uint32_t adc_BB1;
uint32_t adc_BC;
uint32_t adc_CC1;

uint32_t r_AA1;
uint32_t r_AB;
uint32_t r_AC;
uint32_t r_BB1;
uint32_t r_BC;
uint32_t r_CC1;

// adc read with pin low
uint32_t calib_floor;
// adc read with pin 10 ohm to low
uint32_t calib_10_delta;
#define CALIB_10 10


uint32_t covert_to_ohms(uint32_t sample) {
  uint32_t result = ((sample - calib_floor) * CALIB_10 * 10) / (calib_10_delta);
  // if (result > 9999) {
  //   result = 9999;
  // }
  return result;
}

void calibrateLevels() {
  digitalWrite(PIN_TADrv, HIGH);
  pinMode(PIN_TADrv, OUTPUT);

  calib_floor = samplePinWithPinLow(PIN_TA_10, PIN_TA);
  calib_10_delta = samplePinWithPinLow(PIN_TA, PIN_TA_10) - calib_floor;

  pinMode(PIN_TADrv, INPUT);
  pinMode(PIN_TA, INPUT);
  pinMode(PIN_TA_10, INPUT);

}

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

  samplePins();

  Serial.printf("raw adc_BB1: %4d\n", adc_BB1);

  r_AA1 = covert_to_ohms(adc_AA1);
  r_AB = covert_to_ohms(adc_AB);
  r_AC = covert_to_ohms(adc_AC);
  r_BB1 = covert_to_ohms(adc_BB1);
  r_BC = covert_to_ohms(adc_BC);
  r_CC1 = covert_to_ohms(adc_CC1);

 // double height.
  display.setTextSize(1,2);
// erase as we write text
  display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
  //display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);

  display.printf("A %4d  B %4d C %4d\n", r_AA1, r_BB1, r_CC1);
  display.printf("AB%4d AC %4d BC%4d\n", r_AB, r_AC, r_BC);
  // display.printf("A %4d  B %4d C %4d\n", adc_AA1, adc_BB1, adc_CC1);
  // display.printf("AB%4d AC %4d BC%4d\n", adc_AB, adc_AC, adc_BC);

  display.display();

}


void samplePins() {

  digitalWrite(PIN_TADrv, HIGH);
  pinMode(PIN_TADrv, OUTPUT);

  adc_AA1 = samplePinWithPinLow(PIN_TA, PIN_TA1);
  adc_AB = samplePinWithPinLow(PIN_TA, PIN_TB);
  adc_AC = samplePinWithPinLow(PIN_TA, PIN_TC);

  pinMode(PIN_TADrv, INPUT);

  digitalWrite(PIN_TBDrv, HIGH);
  pinMode(PIN_TBDrv, OUTPUT);

  adc_BB1 = samplePinWithPinLow(PIN_TB, PIN_TB1);
  adc_BC = samplePinWithPinLow(PIN_TB, PIN_TC);

  pinMode(PIN_TBDrv, INPUT);

  digitalWrite(PIN_TCDrv, HIGH);
  pinMode(PIN_TCDrv, OUTPUT);

  adc_CC1 = samplePinWithPinLow(PIN_TC, PIN_TC1);

  pinMode(PIN_TCDrv, INPUT);

}


