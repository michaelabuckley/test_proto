#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "adc.h"
#include "oled.h"

//char *button_display = " ";


const int delay_ms = 500;
int count = 0;
#define PIN_D5 (5u);
int p_test = 5;
float r_ref = 3570.0F;
// char adc_bits = 12;
// float adc_scale = 4095.0F;
char adc_bits = 10;
float adc_scale = 1023.0F;
int adc_gain = 1;
//int adc_gain = 16;


void setup() {
  //Serial.begin(9600);
  Serial.println("initDisplay");

  initDisplay();

  // setup buttons
  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);

  Serial.println("IO test");

  // text display tests
  display.setTextSize(1,2);
  // SSD1306_INVERSE
  // display.setTextColor(SSD1306_WHITE);
  // display.setCursor(0,0);

  // display.println("A y.x   B 1.2   C 1.2");
  // display.println("   AB 1.2   BC 1.2");
  // display.print("Connecting to SSID\n'adafruit':");
  // display.print("connected!");
  // display.println("IP: 10.0.1.23");
  // display.println("Sending val #0");
  display.setCursor(0,0);
  display.display(); // actually display all of the above

  analogSetup();
}

void analogSetup() {
  analogReadResolution(adc_bits);
  analogReference(AR_EXTERNAL);
  if (adc_gain == 1) {
    ADC->INPUTCTRL.bit.GAIN = ADC_INPUTCTRL_GAIN_1X_Val;
  } else if (adc_gain == 2) {
    ADC->INPUTCTRL.bit.GAIN = ADC_INPUTCTRL_GAIN_2X_Val;
  } else if (adc_gain == 4) {
    ADC->INPUTCTRL.bit.GAIN = ADC_INPUTCTRL_GAIN_4X_Val;
  } else if (adc_gain == 8) {
    ADC->INPUTCTRL.bit.GAIN = ADC_INPUTCTRL_GAIN_8X_Val;
  } else if (adc_gain == 16) {
    ADC->INPUTCTRL.bit.GAIN = ADC_INPUTCTRL_GAIN_16X_Val;
  }
}

float r_x_display = 42.2;
int s = 18;

void loop() {
  bool enabled = false;
  bool value = false;
  if(!digitalRead(BUTTON_A)) {
    //display.print("A");
    enabled = true;
    value = 1;
  }
  if(!digitalRead(BUTTON_B)) {
    //display.print("B");
    enabled = true;
    value = false;
  }
  //if(!digitalRead(BUTTON_C)) display.print("C");
  // delay(10);
  // yield();
  if (enabled) {
    pinMode(PIN_A4, OUTPUT);
    digitalWrite(PIN_A4, value);
  } else {
    pinMode(PIN_A4, INPUT);
  }


  Serial.println("loop()");
  Serial.printf("Hello %d d \n", s);
  Serial.print(r_x_display, 3);           
  Serial.printf(" f %.2f \n", r_x_display);

  uint32_t adc_read = analogRead(PIN_A0);
  // float adc_norm = (adc_read/adc_scale)/adc_gain;
  // float r_x = r_ref * (adc_norm / (1.0 - adc_norm));
  // r_x_display = (r_x + r_x_display) / 2;
  // // Serial.println(r_x);
  Serial.printf("raw adc: %d\n", adc_read);


  // display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
  // display.setTextSize(1,2);
  // display.setCursor(0,0);
  // display.clearDisplay();

  // display.printf("A %2.2f \n", r_x_display);
  // display.println("   AB xxx   BC xxx");

  // display.println("A 2.2   B 1.2   C 1.2");
  // display.println("   AB 1.2   BC 1.2");

// erase as we write text
  display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
  //display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);

  display.printf("A %d \n", adc_read);
  display.println("   AB 1.2   BC 1.2");

  display.display();


}


