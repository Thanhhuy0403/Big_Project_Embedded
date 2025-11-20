#ifndef __TASK_SEND_CONFIG_TO_FIREBASE__
#define __TASK_SEND_CONFIG_TO_FIREBASE__

#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>

#include "global.h"
#include "taskConnectToFirebase.h"

// Task để gửi tất cả cấu hình của ESP32 lên Firebase
// Bao gồm: SSID, Password WiFi, Device ID, Send Interval, IP Address, v.v.
void taskSendConfigToFirebase(void* pvParameters);

// Hàm gửi config lên Firebase
bool sendConfigToFirebase();

#endif

