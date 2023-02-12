#include <SPI.h>
#include <Wire.h>

void analogSetup();

uint32_t analogReadOversample(uint32_t pin, char samples);
