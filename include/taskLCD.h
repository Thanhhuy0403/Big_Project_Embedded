#ifndef __TASK_LCD__
#define __TASK_LCD__
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

#include "global.h"

void taskLCD(void* pvParameters);

#endif