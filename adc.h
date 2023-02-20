#include <SPI.h>
#include <Wire.h>

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


void analogSetup();

void take_sample();


uint32_t analogReadOversample(uint32_t pin, char samples);
uint32_t samplePinWithPinLow(uint32_t samplePin, uint32_t lowPin);
void samplePins(Sample *sample);
void convert_to_ohms(Sample *in_adc, Sample *out_r);
void calibrateLevels();
