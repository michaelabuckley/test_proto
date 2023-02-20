#include <SPI.h>
#include <Wire.h>

void analogSetup();

uint32_t analogReadOversample(uint32_t pin, char samples);
uint32_t samplePinWithPinLow(uint32_t samplePin, uint32_t lowPin);
void samplePins();

