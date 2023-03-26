#ifdef __IN_ECLIPSE__
//This is a automatic generated file
//Please do not modify this file
//If you touch this file your change will be overwritten during the next build
//This file has been generated on 2023-03-25 23:39:33

#include "Arduino.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "board_wiring.h"
#include "adc.h"
#include "oled.h"
#include "sample.h"

void pinSetup() ;
void setup() ;
void format_fixed(char *out, int32_t val) ;
void draw_all_samples_fixed(Sample *r) ;
inline int32_t clamp(int32_t val) ;
void draw_all_samples(Sample *r) ;
void display_msg(const char *s) ;
void draw_msg(const char *s) ;
void draw_calibration() ;
void change_mode() ;
void display_current_mode() ;
void loop() ;

#include "test_proto.ino"


#endif
