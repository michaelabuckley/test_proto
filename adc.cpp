#include "adc.h"

const char adc_bits = 12;
const char adc_bits_norm = 13;
float adc_scale = 4095.0F;
//char adc_bits = 10;
//float adc_scale = 1023.0F;
//int adc_gain = 1;
int adc_gain = 8;
//int adc_gain = 16;


const char adc_norm_scale = 1ul<<(adc_bits_norm - adc_bits);


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

uint32_t analogReadOversample(uint32_t pin, char samples) {
  uint32_t adc_read = 0;
  for (int i=0; i < samples * adc_norm_scale ; ++i) {
  adc_read += (analogRead(pin));
  }
  adc_read /= samples;
  return adc_read;
}


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



