#ifndef __TASK_WATER__
#define __TASK_WATER__
#include <Arduino.h>

#include "global.h"

#define PUMP_PIN_1 10  // GPIO pin for output 1 (pump)
#define PUMP_PIN_2 17  // GPIO pin for output 2 (reserved for future use)
#define BUTTON_PIN 8   // GPIO pin for button control

void taskWater(void* pvParameters);

#endif

