#include "global.h"
#include "taskTempHumi.h"

void setup() {
    Serial.begin(115200);
    xTaskCreate(taskTempHumi, "Temp and Humi", 2000, NULL, 0, NULL);
}

void loop() {}
