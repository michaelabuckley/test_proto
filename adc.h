#include <SPI.h>
#include <Wire.h>

// 6 samples we collect.  ADC samples are < 16 bits.  int32 is large enough to do sums and conversions
#define SAMPLE_COMPONENT_COUNT (6)
typedef int32_t sample_t;
typedef union {
  struct {
    sample_t sAA1;
    sample_t sAB;
    sample_t sAC;
    sample_t sBB1;
    sample_t sBC;
    sample_t sCC1;
  };
  sample_t component[SAMPLE_COMPONENT_COUNT];
} Sample;


extern Sample resistance_avg;
extern Sample resistance_max;
extern Sample adc_avg;
extern Sample adc_max;

void analogSetup();

void take_sample();


uint32_t analogReadOversample(uint32_t pin, char samples);
uint32_t samplePinWithPinLow(uint32_t samplePin, uint32_t lowPin);
void samplePins(Sample *sample);
void convert_to_ohms(Sample *in_adc, Sample *out_r);
void calibrateLevels();
void external_calibration();
