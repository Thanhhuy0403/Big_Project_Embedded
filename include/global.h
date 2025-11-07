#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <Arduino.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// ============================================
// SENSOR DATA
// ============================================
extern float glob_temperature;
extern float glob_humidity;
extern bool glob_pump_state;

// ============================================
// REMOTE PUMP CONTROL
// ============================================
extern bool glob_remote_pump_enabled;  // Bật/tắt điều khiển máy bơm từ xa
extern String glob_remote_pump_state;  // Trạng thái máy bơm từ Firebase (ON/OFF)

// ============================================
// REMOTE LED CONTROL
// ============================================
extern bool glob_remote_led_enabled;  // Bật/tắt điều khiển LED từ xa
extern String glob_remote_led_state;  // Trạng thái LED từ Firebase (ON/OFF)

// ============================================
// DEVICE CONFIGURATION
// ============================================
extern String glob_device_id;
extern unsigned long glob_send_interval;  // Thời gian gửi dữ liệu lên Firebase (ms)

// ============================================
// WIFI CONFIGURATION
// ============================================
extern String wifi_ssid;
extern String wifi_password;
extern bool wifi_configured;

// ============================================
// DEFAULT VALUES
// ============================================
#ifndef DEFAULT_DEVICE_ID
#define DEFAULT_DEVICE_ID "YoloUno-001"
#endif

#ifndef DEFAULT_SEND_INTERVAL
#define DEFAULT_SEND_INTERVAL 10000  // 10 giây (ms)
#endif

#ifndef DEFAULT_AP_SSID
#define DEFAULT_AP_SSID "YoloUno-Setup-Thanhhuy"
#endif

#ifndef DEFAULT_AP_PASSWORD
#define DEFAULT_AP_PASSWORD "12345678"
#endif

#endif