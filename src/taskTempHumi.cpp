#include "taskTempHumi.h"

float glob_temperature = 1;
float glob_humidity = 1;

DHT20 DHT(&Wire1);

void taskTempHumi(void* pvParameters) {
    Wire1.begin(11, 12);
    delay(2000);
    while (1) {
        DHT.read();
        Serial.print(DHT.getHumidity(), 1);
        Serial.print(",\t");
        Serial.println(DHT.getTemperature(), 1);
        vTaskDelay(5000);
    }
}