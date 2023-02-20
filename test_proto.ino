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


int32_t r_AA1;
int32_t r_AB;
int32_t r_AC;
int32_t r_BB1;
int32_t r_BC;
int32_t r_CC1;

// adc read with pin low
int32_t calib_floor;
// adc read with pin 10 ohm to low
int32_t calib_10_delta;
#define CALIB_10 10

// 6 samples we collect.  ADC samples are < 16 bits.  int32 is large enough to do sums
typedef union {
  struct {
    int32_t sAA1;
    int32_t sAB;
    int32_t sAC;
    int32_t sBB1;
    int32_t sBC;
    int32_t sCC1;
  };
  int32_t sample[6];
} Sample;

Sample adc = {0};

int32_t covert_to_ohms(uint32_t sample) {

  int32_t result = ((((int32_t)sample) - calib_floor) * CALIB_10 * 10) / (calib_10_delta);
  if (result > 9999) {
    result = 9999;
  }
  return result;
}

void calibrateLevels() {
  pinMode(PIN_TADrv, OUTPUT);
  digitalWrite(PIN_TADrv, HIGH);
  delay(1);

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

  samplePins(&adc);

  //Serial.printf("raw adc_BB1: %4d\n", adc_BB1);

  r_AA1 = covert_to_ohms(adc.sAA1);
  r_AB = covert_to_ohms(adc.sAB);
  r_AC = covert_to_ohms(adc.sAC);
  r_BB1 = covert_to_ohms(adc.sBB1);
  r_BC = covert_to_ohms(adc.sBC);
  r_CC1 = covert_to_ohms(adc.sCC1);

  if (r_BB1 > 1000) {
  Serial.printf("raw adc.sBB1: %4d\n", adc.sBB1);
  Serial.printf("raw r_BB1: %4d\n", r_BB1);
  Serial.printf("calib: %4d %4d\n", calib_floor, calib_10_delta);
  }

 // double height.
  display.setTextSize(1,2);
// erase as we write text
  display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
  //display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);

  display.printf("A %4d  B %4d C %4d\n", r_AA1, r_BB1, r_CC1);
  display.printf("AB%4d AC %4d BC%4d\n", r_AB, r_AC, r_BC);
  // display.printf("A %4d  B %4d C %4d\n", adc.AA1, adc.BB1, adc.CC1);
  // display.printf("AB%4d AC %4d BC%4d\n", adc.AB, adc.AC, adc.BC);

  display.display();

}


void samplePins(Sample *sample) {

  digitalWrite(PIN_TADrv, HIGH);
  pinMode(PIN_TADrv, OUTPUT);

  sample->sAA1 = samplePinWithPinLow(PIN_TA, PIN_TA1);
  sample->sAB = samplePinWithPinLow(PIN_TA, PIN_TB);
  sample->sAC = samplePinWithPinLow(PIN_TA, PIN_TC);

  pinMode(PIN_TADrv, INPUT);

  digitalWrite(PIN_TBDrv, HIGH);
  pinMode(PIN_TBDrv, OUTPUT);

  sample->sBB1 = samplePinWithPinLow(PIN_TB, PIN_TB1);
  sample->sBC = samplePinWithPinLow(PIN_TB, PIN_TC);

  pinMode(PIN_TBDrv, INPUT);

  digitalWrite(PIN_TCDrv, HIGH);
  pinMode(PIN_TCDrv, OUTPUT);

  sample->sCC1 = samplePinWithPinLow(PIN_TC, PIN_TC1);

  pinMode(PIN_TCDrv, INPUT);

}


