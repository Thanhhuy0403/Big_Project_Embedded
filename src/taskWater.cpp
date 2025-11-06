#include "taskWater.h"

#include "global.h"

bool glob_pump_state = false;

void taskWater(void* pvParameters) {
    const int ledcChannel = 0;
    const int ledcFrequency = 5000;
    const int ledcResolution = 8;
    ledcSetup(ledcChannel, ledcFrequency, ledcResolution);
    ledcAttachPin(PUMP_PIN_1, ledcChannel);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    const int pumpPower = (int)(255 * 0.70);
    bool lastButtonState = HIGH;
    bool buttonPressed = false;
    unsigned long buttonPressTime = 0;
    bool manualMode = false;
    unsigned long shortPressStartTime = 0;
    bool shortPressActive = false;

    while (1) {
        bool currentButtonState = digitalRead(BUTTON_PIN);
        if (currentButtonState == LOW && lastButtonState == HIGH) {
            buttonPressed = true;
            buttonPressTime = millis();
            Serial.println("Button pressed");
        }
        if (currentButtonState == HIGH && lastButtonState == LOW) {
            unsigned long pressDuration = millis() - buttonPressTime;

            if (buttonPressed) {
                if (pressDuration < 1000) {
                    Serial.println("Short press - Pump ON for 2s");
                    ledcWrite(ledcChannel, pumpPower);
                    glob_pump_state = true;
                    shortPressActive = true;
                    shortPressStartTime = millis();
                } else {
                    Serial.println("Long press released - Pump OFF");
                    ledcWrite(ledcChannel, 0);
                    glob_pump_state = false;
                    manualMode = false;
                }
                buttonPressed = false;
            }
        }
        if (currentButtonState == LOW && buttonPressed) {
            unsigned long pressDuration = millis() - buttonPressTime;
            if (pressDuration >= 1000 && !manualMode) {
                manualMode = true;
                Serial.println("Long press - Pump ON (manual mode)");
                ledcWrite(ledcChannel, pumpPower);
                glob_pump_state = true;
            }
        }
        if (shortPressActive) {
            unsigned long elapsed = millis() - shortPressStartTime;
            if (elapsed >= 2000) {
                ledcWrite(ledcChannel, 0);
                glob_pump_state = false;
                shortPressActive = false;
                Serial.println("Short press duration ended - Pump OFF");
            }
        }

        lastButtonState = currentButtonState;
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}
