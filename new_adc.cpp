/* Copied from wiring_analog.c *
/*
  Copyright (c) 2014 Arduino LLC.  All right reserved.
  SAMD51 support added by Adafruit - Copyright (c) 2018 Dean Miller for Adafruit Industries

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "Arduino.h"
#include "wiring_private.h"

#ifdef __cplusplus
extern "C" {
#endif


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


uint32_t analogReadNew(uint32_t pin)
{
  Serial.printf("analogReadNew: %d\n", pin);

  uint32_t valueRead = 0;

#if defined(PIN_A6)
  if (pin == 6) {
    pin = PIN_A6;
  } else
#endif
#if defined(PIN_A7)
  if (pin == 7) {
    pin = PIN_A7;
  } else 
#endif
  if (pin <= 5) {
    pin += A0;
  }

  pinPeripheral(pin, PIO_ANALOG);
 //ATSAMR, for example, doesn't have a DAC

  // SAMD21 CODE 
  syncADC();
  ADC->INPUTCTRL.bit.MUXPOS = g_APinDescription[pin].ulADCChannelNumber; // Selection for the positive ADC input
  
  // Control A
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

#ifdef __cplusplus
}
#endif
