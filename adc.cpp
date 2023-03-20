#include "Arduino.h"
#include "wiring_private.h"

#include "wiring_analog.h"
#include "board_wiring.h"
#include "adc.h"

uint32_t analogReadOversample(uint32_t pin, char samples);
uint32_t samplePinWithPinLow(uint32_t samplePin, uint32_t lowPin);
void samplePins(Sample *sample);
void samplePinsNew(Sample *sample);
void convert_to_ohms(Sample *in_adc, Sample *out_r);



/*
 * \brief Reads the value from the specified analog pin.
 *
 * \param ulPin
 *
 * \return Read value from selected pin, if no error.
 */
uint32_t analogReadNew( uint32_t ulPin ) ;

/*
 * \brief Set the resolution of analogRead return values. Default is 10 bits (range from 0 to 1023).
 *
 * \param res
 */
void analogReadResolutionNew(int res);


const char adc_bits = 10;
const char adc_bits_norm = 10;
//char adc_bits = 10;

//int adc_gain = 1;
const int adc_gain = 1;
//int adc_gain = 16;
const int adc_oversample = 1;
const char adc_norm_scale = 1ul<<(adc_bits_norm - adc_bits);

// adc read with pin low
int32_t calib_floor = 0;
// adc read with pin 10 ohm to low
int32_t calib_10_delta = 200;
#define CALIB_10 10

Sample adc = {0};
Sample resistance_avg = {0};
Sample resistance_max = {0};

#define ADC_SAMPLE_COUNT (4)

int current_adc_sample = 0;
Sample adc_samples[ADC_SAMPLE_COUNT];
Sample adc_avg;
Sample adc_max;


#ifndef _SAMD21_ADC_COMPONENT_
#error "SAMD21 only"
#endif

static int _readResolution = 10;
static int _ADCResolution = 10;

// Wait for synchronization of registers between the clock domains
static __inline__ void syncADC() __attribute__((always_inline, unused));
static void syncADC() {
  while (ADC->STATUS.bit.SYNCBUSY == 1)
    ;
}


static inline uint32_t mapResolution(uint32_t value, uint32_t from, uint32_t to)
{
  if (from == to) {
    return value;
  }
  if (from > to) {
    return value >> (from-to);
  }
  return value << (to-from);
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
    calib_floor = new_floor;
  }
}

void take_sample() {
  Sample *s = &adc_samples[current_adc_sample];
  samplePinsNew(s);
  check_floor(s);
  current_adc_sample = (current_adc_sample + 1) % ADC_SAMPLE_COUNT;

  smooth_samples();

  convert_to_ohms(&adc_avg, &resistance_avg);
  convert_to_ohms(&adc_max, &resistance_max);
}


void analogSetup() {
  analogReadResolutionNew(adc_bits);
  analogReference(AR_EXTERNAL);
  //analogReference(AR_DEFAULT);
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
  adc_read += (analogReadNew(pin));
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
//  pinMode(PIN_TADrv, OUTPUT);
//  digitalWrite(PIN_TADrv, HIGH);
//  delay(1);

//  calib_floor = samplePinWithPinLow(PIN_TA_10, PIN_TA);
//  calib_10_delta = samplePinWithPinLow(PIN_TA, PIN_TA_10) - calib_floor;

  pinMode(PIN_TADrv, INPUT);
  pinMode(PIN_TA, INPUT);
//  pinMode(PIN_TA_10, INPUT);

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

void rawAdcEnable() {
  /*
   * Bit 1 ENABLE: Enable
   *   0: The ADC is disabled.
   *   1: The ADC is enabled.
   * Due to synchronization, there is a delay from writing CTRLA.ENABLE until the peripheral is enabled/disabled. The
   * value written to CTRL.ENABLE will read back immediately and the Synchronization Busy bit in the Status register
   * (STATUS.SYNCBUSY) will be set. STATUS.SYNCBUSY will be cleared when the operation is complete.
   *
   * Before enabling the ADC, the asynchronous clock source must be selected and enabled, and the ADC reference must be
   * configured. The first conversion after the reference is changed must not be used.
   */
  syncADC();
  ADC->CTRLA.bit.ENABLE = 0x01;             // Enable ADC

}


//	  rawAdcEnable();
//
//  // Samples from A
//  pinMode(PIN_TADrv, OUTPUT);
//  digitalWrite(PIN_TADrv, HIGH);
//
//  pinPeripheral(PIN_TA, PIO_ANALOG);
//
//
//  ADC->INPUTCTRL.bit.MUXPOS = g_APinDescription[PIN_TA].ulADCChannelNumber; // Selection for the positive ADC input
//
//
//  // Sample A-A1
//  pinMode(PIN_TA1, OUTPUT);
//  digitalWrite(PIN_TA1, LOW);
//  sample->sAA1 = analogReadNew(PIN_TA);
//
//
////////
//
//
//  // SAMD21 CODE
//
//
//  // Start conversion
//  syncADC();
//  ADC->SWTRIG.bit.START = 1;
//
//  // Waiting for the 1st conversion to complete
//  while (ADC->INTFLAG.bit.RESRDY == 0);
//
//  // Clear the Data Ready flag
//  ADC->INTFLAG.reg = ADC_INTFLAG_RESRDY;
//
//  // Start conversion again, since The first conversion after the reference is changed must not be used.
//  syncADC();
//  ADC->SWTRIG.bit.START = 1;
//
//  // Store the value
//  while (ADC->INTFLAG.bit.RESRDY == 0);   // Waiting for conversion to complete
//  sample->sAA1 = mapResolution(ADC->RESULT.reg, _ADCResolution, _readResolution);
//
//  syncADC();
//  ADC->CTRLA.bit.ENABLE = 0x00;             // Disable ADC
//  syncADC();
//  pinMode(PIN_TA1, INPUT); // release
//
///////

uint32_t samplePairWithPinLow(uint32_t highDiffPin, uint32_t lowDiffPin, uint32_t lowDrivePin) {
  return samplePinWithPinLow(highDiffPin, lowDrivePin);
}



void samplePinsNew(Sample *sample) {
  digitalWrite(PIN_TADrv, HIGH);
  pinMode(PIN_TADrv, OUTPUT);

  sample->sAA1 = samplePairWithPinLow(PIN_TA, PIN_TA1, PIN_TA1Drv);
  sample->sAB = samplePairWithPinLow(PIN_TA, PIN_TB, PIN_TBDrv);
  sample->sAC = samplePairWithPinLow(PIN_TA, PIN_TC, PIN_TCDrv);

  pinMode(PIN_TADrv, INPUT);

  digitalWrite(PIN_TBDrv, HIGH);
  pinMode(PIN_TBDrv, OUTPUT);

  sample->sBB1 = samplePinWithPinLow(PIN_TB, PIN_TB1Drv);
  sample->sBC = samplePinWithPinLow(PIN_TB, PIN_TCDrv);

  pinMode(PIN_TBDrv, INPUT);

  digitalWrite(PIN_TCDrv, HIGH);
  pinMode(PIN_TCDrv, OUTPUT);

  sample->sCC1 = samplePinWithPinLow(PIN_TC, PIN_TC1Drv);

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


// // Wait for synchronization of registers between the clock domains
// static __inline__ void syncTC_16(Tc* TCx) __attribute__((always_inline, unused));
// static void syncTC_16(Tc* TCx) {
//   while (TCx->COUNT16.STATUS.bit.SYNCBUSY);
// }

// // Wait for synchronization of registers between the clock domains
// static __inline__ void syncTCC(Tcc* TCCx) __attribute__((always_inline, unused));
// static void syncTCC(Tcc* TCCx) {
//   while (TCCx->SYNCBUSY.reg & TCC_SYNCBUSY_MASK);
// }

void analogReadResolutionNew(int res)
{
  analogReadResolution(res);
  _readResolution = res;

	if (res > 10) {
		ADC->CTRLB.bit.RESSEL = ADC_CTRLB_RESSEL_12BIT_Val;
		_ADCResolution = 12;
		} else if (res > 8) {
		ADC->CTRLB.bit.RESSEL = ADC_CTRLB_RESSEL_10BIT_Val;
		_ADCResolution = 10;
		} else {
		ADC->CTRLB.bit.RESSEL = ADC_CTRLB_RESSEL_8BIT_Val;
		_ADCResolution = 8;
	}

  syncADC();
}

uint32_t analogReadNew(uint32_t pin)
{
 
  uint32_t valueRead = 0;

  pinPeripheral(pin, PIO_ANALOG);
 
  // SAMD21 CODE 
  syncADC();
  ADC->INPUTCTRL.bit.MUXPOS = g_APinDescription[pin].ulADCChannelNumber; // Selection for the positive ADC input
  
  rawAdcEnable();
  // Start conversion
  syncADC();
  ADC->SWTRIG.bit.START = 1;

  // Waiting for the 1st conversion to complete
  while (ADC->INTFLAG.bit.RESRDY == 0);

  // Clear the Data Ready flag
  ADC->INTFLAG.reg = ADC_INTFLAG_RESRDY;

  // Start conversion again, since The first conversion after the reference is changed must not be used.
  syncADC();
  ADC->SWTRIG.bit.START = 1;

  // Store the value
  while (ADC->INTFLAG.bit.RESRDY == 0);   // Waiting for conversion to complete
  valueRead = ADC->RESULT.reg;

  syncADC();
  ADC->CTRLA.bit.ENABLE = 0x00;             // Disable ADC
  syncADC();

  return mapResolution(valueRead, _ADCResolution, _readResolution);
}


