#include "Arduino.h"

#include <SPI.h>
#include <Wire.h>

#include "sample.h"

#include "board_wiring.h"

#include "rawAdc.h"


void samplePins(Sample *sample);
void samplePinsNew(Sample *sample);
void convert_to_ohms(Sample *in_adc, Sample *out_r);
void check_floor(Sample *sample);
void smooth_samples();
uint32_t samplePinWithPinLow(uint32_t samplePin, uint32_t lowPin);


const char adc_bits = 14;
const char adc_bits_norm = 10;

Sample adc = {0};
Sample resistance_avg = {0};
Sample resistance_max = {0};

#define ADC_SAMPLE_COUNT (4)

int current_adc_sample = 0;
Sample adc_samples[ADC_SAMPLE_COUNT];
Sample adc_avg;
Sample adc_max;

// adc read with pins A/A1 short
int32_t calib_floor = -15;
// adc read with pin 10 ohm to low
int32_t calib_10_delta = 80;


void sampleSetup() {
  analogCalibrate();
  analogReference2(ADC_REF_INT1V);
  analogReadExtended(adc_bits);
  analogGain(ADC_GAIN_2);
  analogReferenceCompensation(1);
}

void take_sample() {
  Sample *s = &adc_samples[current_adc_sample];
  samplePinsNew(s);
  //samplePins(s);
  check_floor(s);
  current_adc_sample = (current_adc_sample + 1) % ADC_SAMPLE_COUNT;

  smooth_samples();

  convert_to_ohms(&adc_avg, &resistance_avg);
  convert_to_ohms(&adc_max, &resistance_max);
}

void smooth_samples() {
  adc_avg = (const Sample){ 0 };
  adc_max = (const Sample){ 0 };
  for (int i=0; i<ADC_SAMPLE_COUNT; i+=1) {
    for (int j=0; j<SAMPLE_COMPONENT_COUNT; j+=1) {
      sample_t v = adc_samples[i].component[j];
      adc_avg.component[j] += v;

      sample_t cur_max = adc_max.component[j];
      if (v > cur_max) {
        adc_max.component[j] = v;
      }
    }
  }
  for (int j=0; j<6; j+=1) {
    adc_avg.component[j] /= ADC_SAMPLE_COUNT;
  }
}

void check_floor(Sample *sample) {
  sample_t new_floor = min(min(min(min(min(
       sample->sAA1,
       sample->sAB),
       sample->sAC),
       sample->sBB1),
       sample->sBC),
       sample->sCC1);
  if (new_floor < calib_floor) {
    Serial.printf("Floor change from %d to %d\n", calib_floor, new_floor);
    //calib_floor = new_floor;
  }
}

/**
 * Use current AA1 as 0.
 */
void external_calibration() {
  int32_t old_floor;
  old_floor = calib_floor;
  calib_floor = adc_avg.sAA1;
  Serial.printf("external_calibration: %d to %d\n", old_floor, calib_floor);
}


// convert to ohms with fixed e-1
int32_t covert_to_ohms(uint32_t sample) {

  int32_t result = ((((int32_t)sample) - calib_floor) * 10 * 10) / (calib_10_delta);
  //int32_t result = ((((int32_t)sample) - calib_floor) ) / (calib_10_delta);
  return result;
}

void convert_to_ohms(Sample *in_adc, Sample *out_r) {
  out_r->sAA1 = covert_to_ohms(in_adc->sAA1);
  out_r->sAB = covert_to_ohms(in_adc->sAB);
  out_r->sAC = covert_to_ohms(in_adc->sAC);
  out_r->sBB1 = covert_to_ohms(in_adc->sBB1);
  out_r->sBC = covert_to_ohms(in_adc->sBC);
  out_r->sCC1 = covert_to_ohms(in_adc->sCC1);

  if (out_r->sBB1 > 9999) {
    Serial.printf("raw adc.sBB1: %4d\n", in_adc->sBB1);
    Serial.printf("raw r_BB1: %4d\n", out_r->sBB1);
    Serial.printf("calib: %4d %4d\n", calib_floor, calib_10_delta);
  }

}

uint32_t samplePairWithPinLow(uint32_t highDiffPin, uint32_t lowDiffPin, uint32_t lowDrivePin) {

  pinMode(lowDrivePin, OUTPUT);
  digitalWrite(lowDrivePin, LOW);
  delay(1);
  // Serial.printf("Set       %d low \n", lowPin);
  // Serial.printf("Readback  %d %d \n", lowPin, digitalRead(lowPin));

  uint32_t result = analogDifferential(highDiffPin, lowDiffPin);

  pinMode(lowDrivePin, INPUT);

  return result;
}


void samplePinsNew(Sample *sample) {
  //rawAdcEnable();

  pinMode(PIN_TADrv, OUTPUT);
  digitalWrite(PIN_TADrv, HIGH);

  sample->sAA1 = samplePairWithPinLow(PIN_TA, PIN_TA1, PIN_TA1Drv);
  sample->sAB  = samplePairWithPinLow(PIN_TA, PIN_TB, PIN_TBDrv);
  sample->sAC  = samplePairWithPinLow(PIN_TA, PIN_TC, PIN_TCDrv);

  pinMode(PIN_TADrv, INPUT);

  pinMode(PIN_TBDrv, OUTPUT);
  digitalWrite(PIN_TBDrv, HIGH);

  sample->sBB1 = samplePairWithPinLow(PIN_TB, PIN_TB1, PIN_TB1Drv);
  sample->sBC  = samplePairWithPinLow(PIN_TB, PIN_TC, PIN_TCDrv);

  pinMode(PIN_TBDrv, INPUT);

  pinMode(PIN_TC1Drv, OUTPUT);
  digitalWrite(PIN_TC1Drv, HIGH);
  delay(2);

  sample->sCC1 = samplePairWithPinLow(PIN_TC1, PIN_TC, PIN_TCDrv);

  pinMode(PIN_TC1Drv, INPUT);

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


