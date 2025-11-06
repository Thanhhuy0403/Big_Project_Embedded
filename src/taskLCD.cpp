#include "taskLCD.h"
LiquidCrystal_I2C lcd(0x21, 16, 2);
void taskLCD(void* pvParameters) {
    Wire.begin();
    delay(500);
    lcd.begin();
    delay(100);
    lcd.backlight();
    delay(100);
    lcd.clear();
    lcd.setCursor(0, 0);
    delay(2000);
    while (1) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Temp: ");
        lcd.print(glob_temperature, 1);
        lcd.print(" C");
        lcd.setCursor(0, 1);
        lcd.print("Humi: ");
        lcd.print(glob_humidity, 1);
        lcd.print(" %");
        vTaskDelay(5000);
    }
}