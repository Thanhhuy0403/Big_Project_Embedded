#include "taskNeoBlinky.h"

void calculateColorFromTempHumi(float temp, float humi, uint16_t& hue, uint8_t& saturation, uint8_t& brightness) {
    if (temp < -10.0) temp = -10.0;
    if (temp > 50.0) temp = 50.0;
    if (humi < 0) humi = 0;
    if (humi > 100) humi = 100;
    float baseHue;

    if (temp < 10.0) {
        baseHue = 240.0;
    } else if (temp <= 25.0) {
        float ratio = (temp - 10.0) / (25.0 - 10.0);
        baseHue = 240.0 - (ratio * 120.0);
    } else {
        float ratio = (temp - 25.0) / (50.0 - 25.0);
        if (ratio < 0.5) {
            baseHue = 120.0 - (ratio * 2.0 * 60.0);
        } else {
            baseHue = 60.0 - ((ratio - 0.5) * 2.0 * 60.0);
        }
    }
    float humiAdjustment = (humi - 50.0) / 50.0;
    float hueAdjustment = humiAdjustment * 20.0;
    baseHue += hueAdjustment;
    while (baseHue < 0) baseHue += 360;
    while (baseHue >= 360) baseHue -= 360;

    hue = (uint16_t)(baseHue * 256.0 / 360.0);
    saturation = 180 + (uint8_t)((humi / 100.0) * 75);
    float tempBrightness = 150.0 + ((temp + 10.0) / 60.0) * 80.0;
    float humiBrightness = 100.0 + (humi / 100.0) * 100.0;
    brightness = (uint8_t)((tempBrightness + humiBrightness) / 2.0);
    if (brightness > 255) brightness = 255;
    if (brightness < 100) brightness = 100;
}

void taskNeoBlinky(void* pvParameters) {
    Adafruit_NeoPixel strip(LED_COUNT, NEO_PIN, NEO_GRB + NEO_KHZ800);
    strip.begin();
    strip.clear();
    strip.show();
    strip.setBrightness(200);

    while (1) {
        float temp = glob_temperature;
        float humi = glob_humidity;
        uint16_t hue;
        uint8_t saturation;
        uint8_t brightness;
        calculateColorFromTempHumi(temp, humi, hue, saturation, brightness);
        uint32_t color = strip.gamma32(strip.ColorHSV(hue, saturation, brightness));
        for (int i = 0; i < LED_COUNT; i++) {
            strip.setPixelColor(i, color);
        }
        strip.show();
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}