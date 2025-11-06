#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <Arduino.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern float glob_temperature;
extern float glob_humidity;
extern bool glob_pump_state;

#endif