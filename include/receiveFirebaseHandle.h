#ifndef __RECEIVE_FIREBASE_HANDLE__
#define __RECEIVE_FIREBASE_HANDLE__

#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>

#include "global.h"
#include "taskConnectToFirebase.h"  // Để sử dụng FIREBASE_HOST, FIREBASE_AUTH_SECRET, FIREBASE_AUTH_TOKEN

// ============================================
// CẤU HÌNH FETCH DỮ LIỆU TỪ FIREBASE
// ============================================
// Path trong Firebase để fetch dữ liệu điều khiển máy bơm
#define FIREBASE_PUMP_CONTROL_PATH "/realtime-pump"

// Khoảng thời gian giữa các lần fetch dữ liệu (ms)
#define FIREBASE_FETCH_INTERVAL 2000  // 2 giây

// Hàm fetch dữ liệu từ Firebase
bool fetchPumpControlFromFirebase(String& pumpState);

// Task chính để nhận và xử lý dữ liệu từ Firebase
void taskReceiveFirebaseHandle(void* pvParameters);

#endif

