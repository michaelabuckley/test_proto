#include "Arduino.h"
#include "wiring_private.h"

#include "wiring_analog.h"
#include "board_wiring.h"
#include "adc.h"

uint32_t analogReadOversample(uint32_t pin, char samples);
uint32_t samplePinWithPinLow(uint32_t samplePin, uint32_t lowPin);
uint32_t adcRawReadDifferential(uint32_t highDiffPin, uint32_t lowDiffPin);
uint32_t samplePinWithPinLow(uint32_t samplePin, uint32_t lowPin);



/*
 * \brief Reads the value from the specified analog pin.
 *
 * \param ulPin
 *
 * \return Read value from selected pin, if no error.
 */
uint32_t rawAdcAnalogRead( uint32_t ulPin ) ;

/*
 * \brief Set the resolution of analogRead return values. Default is 10 bits (range from 0 to 1023).
 *
 * \param res
 */
void analogReadResolutionNew(int res);


const char adc_bits = 12;
const char adc_bits_norm = 12;
//char adc_bits = 10;

//int adc_gain = 1;
const int adc_gain = 4;
//int adc_gain = 16;
const int adc_oversample = 1;
const char adc_norm_scale = 1ul<<(adc_bits_norm - adc_bits);




#ifndef _SAMD21_ADC_COMPONENT_
#error "SAMD21 only"
#endif

static int _readResolution = 12;
static int _ADCResolution = 12;

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
  adc_read += (rawAdcAnalogRead(pin));
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

  //uint32_t result = analogReadOversample(samplePin, adc_oversample);
  uint32_t result = analogRead(samplePin);
  pinMode(lowPin, INPUT);
  return result;
}



void calibrateLevels() {
  pinMode(PIN_TADrv, OUTPUT);
  digitalWrite(PIN_TADrv, HIGH);
  delay(1);

  // fixme add 10R calibration back in.
  // hacky adc config check -
  //samplePins(&(adc_samples[0]));
  //calib_floor = adcRawReadDifferential(PIN_TA, PIN_TA);
//  calib_floor = samplePinWithPinLow(PIN_TA_10, PIN_TA);
//  calib_10_delta = samplePinWithPinLow(PIN_TA, PIN_TA_10) - calib_floor;

  pinMode(PIN_TADrv, INPUT);
  pinMode(PIN_TA, INPUT);
//  pinMode(PIN_TA_10, INPUT);

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
  syncADC();

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


uint32_t adcRawReadDifferential(uint32_t highDiffPin, uint32_t lowDiffPin) {

  uint32_t valueRead = 0;

  pinPeripheral(highDiffPin, PIO_ANALOG);
  pinPeripheral(lowDiffPin, PIO_ANALOG);

  // SAMD21 CODE
  syncADC();
  ADC->INPUTCTRL.bit.MUXPOS = g_APinDescription[highDiffPin].ulADCChannelNumber; // Selection for the positive ADC input
  syncADC();
  ADC->INPUTCTRL.bit.MUXNEG = g_APinDescription[lowDiffPin].ulADCChannelNumber; // Selection for the positive ADC input


  syncADC();
  ADC->CTRLB.bit.DIFFMODE = 0;
  syncADC();
  ADC->AVGCTRL.bit.SAMPLENUM = ADC_AVGCTRL_SAMPLENUM_4_Val;
  ADC->AVGCTRL.bit.ADJRES = 1;


  // Start conversion -- we assume the reference isn't changed, and we can reuse the setup
  syncADC();

  // Clear the Data Ready flag
  ADC->INTFLAG.reg = ADC_INTFLAG_RESRDY;

  syncADC();
  ADC->SWTRIG.bit.START = 1;

  // Store the value
  while (ADC->INTFLAG.bit.RESRDY == 0);   // Waiting for conversion to complete
  valueRead = ADC->RESULT.reg;

  return mapResolution(valueRead, _ADCResolution, _readResolution);
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

uint32_t rawAdcAnalogRead(uint32_t pin)
{
 
  uint32_t valueRead = 0;

  pinPeripheral(pin, PIO_ANALOG);
 
  // SAMD21 CODE 
  syncADC();
  ADC->INPUTCTRL.bit.MUXPOS = g_APinDescription[pin].ulADCChannelNumber; // Selection for the positive ADC input
  
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

  return mapResolution(valueRead, _ADCResolution, _readResolution);
}


