#include "taskTempHumi.h"

float glob_temperature = 1;
float glob_humidity = 1;

DHT20 DHT(&Wire);

void taskTempHumi(void* pvParameters) {
    Wire.begin(11, 12);
    delay(2000);
    while (1) {
        DHT.read();
        glob_humidity = DHT.getHumidity();
        glob_temperature = DHT.getTemperature();
        Serial.print("Humi: ");
        Serial.print(glob_humidity, 1);
        Serial.print(" %,\tTemp: ");
        Serial.print(glob_temperature, 1);
        Serial.println(" C");
        vTaskDelay(5000);
    }
}