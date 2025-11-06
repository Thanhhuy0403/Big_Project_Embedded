#include "global.h"
#include "taskConnectWifi.h"
#include "taskLCD.h"
#include "taskNeoBlinky.h"
#include "taskTempHumi.h"
#include "taskWebServer.h"

void setup() {
    Serial.begin(115200);
    delay(1000);
    xTaskCreate(taskWebServer, "Web Server", 8192, NULL, 1, NULL);
    delay(500);
    xTaskCreate(taskConnectWifi, "Connect WiFi", 4096, NULL, 1, NULL);
    delay(500);
    xTaskCreate(taskTempHumi, "Temp and Humi", 2000, NULL, 1, NULL);
    // xTaskCreate(taskLCD, "LCD Display", 3000, NULL, 1, NULL);
    // xTaskCreate(taskNeoBlinky, "LED RGB", 2000, NULL, 1, NULL);
}

void loop() {}
