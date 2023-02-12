#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "adc.h"
#include "oled.h"

//char *button_display = " ";


int count = 0;
#define PIN_D5 (5u)
int p_test = 5;
float r_ref = 3570.0F;

#define PIN_TA PIN_A0
#define PIN_TB PIN_A1
#define PIN_TC PIN_A2
#define PIN_TA1 PIN_A3
#define PIN_TB1 PIN_A4
#define PIN_TC1 PIN_A5

void pinSetup() {
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
}


uint32_t adc_AA1;
uint32_t adc_AB;
uint32_t adc_AC;
uint32_t adc_BB1;
uint32_t adc_BC;
uint32_t adc_CC1;


uint32_t samplePinWithPinLow(uint32_t samplePin, uint32_t lowPin) {

  pinMode(lowPin, OUTPUT);
  digitalWrite(lowPin, LOW);
  // sleep(10);
  Serial.printf("Set       %d low \n", lowPin);
  Serial.printf("Readback  %d %d \n", lowPin, digitalRead(lowPin));

  uint32_t result = analogReadOversample(samplePin, 16);
  pinMode(lowPin, INPUT);
  return result;
}

void loop() {
  Serial.println("loop()");

  adc_AA1 = samplePinWithPinLow(PIN_TA, PIN_TA1);
  adc_AB = samplePinWithPinLow(PIN_TA, PIN_TB);
  adc_AC = samplePinWithPinLow(PIN_TA, PIN_TC);
  adc_BB1 = samplePinWithPinLow(PIN_TB, PIN_TB1);
  adc_BC = samplePinWithPinLow(PIN_TB, PIN_TC);
  adc_CC1 = samplePinWithPinLow(PIN_TC, PIN_TC1);

  Serial.printf("raw adc_AA1: %4d\n", adc_AA1);

 // double height.
  display.setTextSize(1,2);
// erase as we write text
  display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
  //display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);

  display.printf("A %4d  B %4d C %4d\n", adc_AA1, adc_BB1, adc_CC1);
  display.printf("AB%4d AC %4d BC%4d\n", adc_AB, adc_AC, adc_BC);

  display.display();


  // if(!digitalRead(BUTTON_A)) {
  //   pinMode(PIN_A3, OUTPUT);
  //   digitalWrite(PIN_A3, LOW);
  // } else {
  //   // pinMode(PIN_A3, INPUT);
  // }
  // if(!digitalRead(BUTTON_B)) {
  //   pinMode(PIN_A4, OUTPUT);
  //   digitalWrite(PIN_A4, LOW);
  // } else {
  //   // pinMode(PIN_A4, INPUT);
  // }
  // if(!digitalRead(BUTTON_C)) {
  //   pinMode(PIN_A5, OUTPUT);
  //   digitalWrite(PIN_A5, LOW);
  // } else {
  //   // pinMode(PIN_A5, INPUT);
  // }
  //if(!digitalRead(BUTTON_C)) display.print("C");
  // delay(10);
  // yield();
  // if (enabled) {
  //   pinMode(PIN_A4, OUTPUT);
  //   digitalWrite(PIN_A4, value);
  // } else {
  //   pinMode(PIN_A4, INPUT);
  // }

}


