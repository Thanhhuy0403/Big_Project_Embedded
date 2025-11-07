#include "taskConnectToFirebase.h"

#include "global.h"
#include <Preferences.h>

HTTPClient http;

bool sendDataToFirebase(float temperature, float humidity) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Firebase: WiFi not connected, cannot send data");
        return false;
    }
    String url = "https://" + String(FIREBASE_HOST) + String(FIREBASE_DB_PATH) + ".json";
    if (strlen(FIREBASE_AUTH_SECRET) > 0) {
        url += "?auth=" + String(FIREBASE_AUTH_SECRET);
    } else if (strlen(FIREBASE_AUTH_TOKEN) > 0) {
        url += "?auth=" + String(FIREBASE_AUTH_TOKEN);
    }
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    String jsonPayload = "{";
    jsonPayload += "\"temperature\":" + String(temperature, 2) + ",";
    jsonPayload += "\"humidity\":" + String(humidity, 2) + ",";
    jsonPayload += "\"timestamp\":" + String(millis() / 1000);
    jsonPayload += "}";

    Serial.println("Firebase: Sending data...");
    Serial.print("URL: ");
    Serial.println(url);
    Serial.print("Payload: ");
    Serial.println(jsonPayload);
    // Sử dụng PATCH thay vì PUT để chỉ cập nhật các field, không ghi đè toàn bộ node
    // PUT sẽ xóa tất cả các node khác ở cùng level, PATCH chỉ cập nhật các field được chỉ định
    // Firebase REST API: PATCH method để cập nhật một phần dữ liệu
    int httpResponseCode = http.sendRequest("PATCH", jsonPayload);
    if (httpResponseCode > 0) {
        Serial.print("Firebase: HTTP Response code: ");
        Serial.println(httpResponseCode);

        String response = http.getString();
        Serial.print("Firebase: Response: ");
        Serial.println(response);

        http.end();

        if (httpResponseCode == 200) {
            Serial.println("Firebase: Data sent successfully!");
            return true;
        } else {
            Serial.print("Firebase: Server returned code: ");
            Serial.println(httpResponseCode);
            return false;
        }
    } else {
        Serial.print("Firebase: Error sending data: ");
        Serial.println(httpResponseCode);
        Serial.print("Error: ");
        Serial.println(http.errorToString(httpResponseCode));
        http.end();
        return false;
    }
}
bool sendDataToFirebaseNested(float temperature, float humidity) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Firebase: WiFi not connected, cannot send data");
        return false;
    }
    String url = "https://" + String(FIREBASE_HOST) + String(FIREBASE_DB_PATH) + "/sensor_data.json";

    if (strlen(FIREBASE_AUTH_SECRET) > 0) {
        url += "?auth=" + String(FIREBASE_AUTH_SECRET);
    } else if (strlen(FIREBASE_AUTH_TOKEN) > 0) {
        url += "?auth=" + String(FIREBASE_AUTH_TOKEN);
    }

    http.begin(url);
    http.addHeader("Content-Type", "application/json");

    String jsonPayload = "{";
    jsonPayload += "\"temperature\":" + String(temperature, 2) + ",";
    jsonPayload += "\"humidity\":" + String(humidity, 2) + ",";
    jsonPayload += "\"timestamp\":" + String(millis() / 1000);
    jsonPayload += "}";

    Serial.println("Firebase: Sending nested data...");
    Serial.print("URL: ");
    Serial.println(url);
    Serial.print("Payload: ");
    Serial.println(jsonPayload);

    // Sử dụng PATCH thay vì PUT để chỉ cập nhật các field, không ghi đè toàn bộ node
    // Firebase REST API: PATCH method để cập nhật một phần dữ liệu
    int httpResponseCode = http.sendRequest("PATCH", jsonPayload);

    if (httpResponseCode > 0) {
        Serial.print("Firebase: HTTP Response code: ");
        Serial.println(httpResponseCode);

        String response = http.getString();
        Serial.print("Firebase: Response: ");
        Serial.println(response);

        http.end();

        if (httpResponseCode == 200) {
            Serial.println("Firebase: Nested data sent successfully!");
            return true;
        } else {
            Serial.print("Firebase: Server returned code: ");
            Serial.println(httpResponseCode);
            return false;
        }
    } else {
        Serial.print("Firebase: Error: ");
        Serial.println(http.errorToString(httpResponseCode));
        http.end();
        return false;
    }
}

void taskConnectToFirebase(void* pvParameters) {
    Serial.println("Task Connect to Firebase: Starting...");
    vTaskDelay(2000 / portTICK_PERIOD_MS);

    // Load device config từ Preferences (bao gồm send_interval)
    Preferences prefs;
    prefs.begin("device_config", true);
    String device_id = prefs.getString("device_id", DEFAULT_DEVICE_ID);
    unsigned long send_interval = prefs.getULong("send_interval", DEFAULT_SEND_INTERVAL);
    prefs.end();
    
    // Cập nhật biến toàn cục với giá trị từ Preferences
    glob_device_id = device_id;
    glob_send_interval = send_interval;
    
    Serial.println("Firebase Task: Loaded configuration:");
    Serial.print("  Device ID: ");
    Serial.println(glob_device_id);
    Serial.print("  Send Interval: ");
    Serial.print(glob_send_interval);
    Serial.println(" ms");

    unsigned long lastSendTime = 0;
    unsigned long lastConfigCheckTime = 0;
    const unsigned long CONFIG_CHECK_INTERVAL = 5000;  // Kiểm tra config mỗi 5 giây

    while (1) {
        if (WiFi.status() == WL_CONNECTED) {
            unsigned long currentTime = millis();
            
            // Kiểm tra và cập nhật send_interval từ Preferences định kỳ
            if (currentTime - lastConfigCheckTime >= CONFIG_CHECK_INTERVAL || lastConfigCheckTime == 0) {
                prefs.begin("device_config", true);
                unsigned long new_interval = prefs.getULong("send_interval", DEFAULT_SEND_INTERVAL);
                prefs.end();
                
                if (new_interval != glob_send_interval && new_interval >= 1000 && new_interval <= 600000) {
                    glob_send_interval = new_interval;
                    Serial.print("Firebase: Send interval updated to ");
                    Serial.print(glob_send_interval);
                    Serial.println(" ms");
                }
                lastConfigCheckTime = currentTime;
            }

            if (currentTime - lastSendTime >= glob_send_interval || lastSendTime == 0) {
                Serial.println("====================");
                Serial.println("Firebase: Preparing to send data...");
                Serial.print("Temperature: ");
                Serial.print(glob_temperature, 2);
                Serial.println(" °C");
                Serial.print("Humidity: ");
                Serial.print(glob_humidity, 2);
                Serial.println(" %");
                bool success = sendDataToFirebase(glob_temperature, glob_humidity);
                if (success) {
                    Serial.println("Firebase: Data sent successfully!");
                } else {
                    Serial.println("Firebase: Failed to send data!");
                    Serial.println("Please check:");
                    Serial.println("1. Firebase Host is correct");
                    Serial.println("2. Database path is correct");
                    Serial.println("3. Authentication (secret/token) is correct");
                    Serial.println("4. Database rules allow write access");
                }
                Serial.println("====================");

                lastSendTime = currentTime;
            }
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        } else {
            Serial.println("Firebase: Waiting for WiFi connection...");
            vTaskDelay(5000 / portTICK_PERIOD_MS);
        }
    }
}
