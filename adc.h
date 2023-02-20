#include <SPI.h>
#include <Wire.h>

// 6 samples we collect.  ADC samples are < 16 bits.  int32 is large enough to do sums and conversions
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


extern Sample display_resistance;

void analogSetup();

void take_sample();


uint32_t analogReadOversample(uint32_t pin, char samples);
uint32_t samplePinWithPinLow(uint32_t samplePin, uint32_t lowPin);
void samplePins(Sample *sample);
void convert_to_ohms(Sample *in_adc, Sample *out_r);
void calibrateLevels();
