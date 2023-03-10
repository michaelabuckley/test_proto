#include "wiring_analog.h"
#include "board_wiring.h"
#include "adc.h"

const char adc_bits = 12;
const char adc_bits_norm = 13;
float adc_scale = 4095.0F;
//char adc_bits = 10;
//float adc_scale = 1023.0F;
//int adc_gain = 1;
const int adc_gain = 4;
//int adc_gain = 16;
const int adc_oversample = 16;



const char adc_norm_scale = 1ul<<(adc_bits_norm - adc_bits);

// adc read with pin low
int32_t calib_floor;
// adc read with pin 10 ohm to low
int32_t calib_10_delta;
#define CALIB_10 10

Sample adc = {0};
Sample avg_resistance = {0};
Sample max_resistance = {0};

#define ADC_SAMPLE_COUNT (4)

int current_adc_sample = 0;
Sample adc_samples[ADC_SAMPLE_COUNT];
Sample adc_avg;
Sample adc_max;

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
    calib_floor = new_floor;
  }
}

void take_sample() {
  Sample *s = &adc_samples[current_adc_sample];
  samplePins(s);
  check_floor(s);
  current_adc_sample = (current_adc_sample + 1) % ADC_SAMPLE_COUNT;

  smooth_samples();

  convert_to_ohms(&adc_avg, &avg_resistance);
  convert_to_ohms(&adc_max, &max_resistance);
}


void analogSetup() {
  analogReadResolution(adc_bits);
  //analogReference(AR_EXTERNAL);
  analogReference(AR_DEFAULT);
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
  delay(1);
  // Serial.printf("Set       %d low \n", lowPin);
  // Serial.printf("Readback  %d %d \n", lowPin, digitalRead(lowPin));

  uint32_t result = analogReadOversample(samplePin, adc_oversample);
  pinMode(lowPin, INPUT);
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

/**
 * Use current AA1 as 0.
 */
void external_calibration() {
  calib_floor = adc_avg.sAA1;
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



int32_t covert_to_ohms(uint32_t sample) {

  int32_t result = ((((int32_t)sample) - calib_floor) * CALIB_10 * 10) / (calib_10_delta);
  if (result > 9999) {
    result = 9999;
  }
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




