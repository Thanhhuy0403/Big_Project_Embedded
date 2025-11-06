#ifndef __TASK_NEO_BLINKY__
#define __TASK_NEO_BLINKY__
#include <Adafruit_NeoPixel.h>
#include <Arduino.h>

#include "global.h"

#define NEO_PIN 6
#define LED_COUNT 4

void taskNeoBlinky(void* pvParameters);

#endif