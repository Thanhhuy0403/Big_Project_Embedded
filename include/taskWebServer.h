#ifndef __TASK_WEB_SERVER__
#define __TASK_WEB_SERVER__
#include <Arduino.h>
#include <WebServer.h>
#include <WiFi.h>

#include "global.h"

#define AP_SSID "YoloUno-Setup-Thanhhuy"
#define AP_PASSWORD "12345678"

extern String wifi_ssid;
extern String wifi_password;
extern bool wifi_configured;

// Hàm test WiFi connection
String testWiFiConnection(String ssid, String password);

void taskWebServer(void* pvParameters);

#endif
