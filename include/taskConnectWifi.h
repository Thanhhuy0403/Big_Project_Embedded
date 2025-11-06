#ifndef __TASK_CONNECT_WIFI__
#define __TASK_CONNECT_WIFI__
#include <Arduino.h>
#include <WiFi.h>

#include "global.h"
#include "taskWebServer.h"  // Để truy cập wifi_ssid và wifi_password

void taskConnectWifi(void* pvParameters);

#endif

