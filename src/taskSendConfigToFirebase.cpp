#include "taskSendConfigToFirebase.h"

#include <Preferences.h>

#include "global.h"
#include "taskConnectToFirebase.h"

HTTPClient httpConfig;

// Hàm gửi tất cả cấu hình của ESP32 lên Firebase
bool sendConfigToFirebase() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Send Config: WiFi not connected, cannot send config");
        return false;
    }

    // Đọc các config từ Preferences
    Preferences prefs;
    prefs.begin("device_config", true);
    String device_id = prefs.getString("device_id", DEFAULT_DEVICE_ID);
    unsigned long send_interval = prefs.getULong("send_interval", DEFAULT_SEND_INTERVAL);
    prefs.end();

    // Tạo URL để gửi config lên Firebase
    // Sử dụng path /config để tách biệt với dữ liệu sensor
    String url = "https://" + String(FIREBASE_HOST) + String(FIREBASE_DB_PATH) + "/config.json";

    if (strlen(FIREBASE_AUTH_SECRET) > 0) {
        url += "?auth=" + String(FIREBASE_AUTH_SECRET);
    } else if (strlen(FIREBASE_AUTH_TOKEN) > 0) {
        url += "?auth=" + String(FIREBASE_AUTH_TOKEN);
    }

    httpConfig.begin(url);
    httpConfig.addHeader("Content-Type", "application/json");

    // Tạo JSON payload chứa tất cả thông tin config
    String jsonPayload = "{";
    
    // WiFi Config
    jsonPayload += "\"wifi\":{";
    jsonPayload += "\"ssid\":\"" + wifi_ssid + "\",";
    jsonPayload += "\"password\":\"" + wifi_password + "\",";
    jsonPayload += "\"configured\":" + String(wifi_configured ? "true" : "false") + ",";
    if (WiFi.status() == WL_CONNECTED) {
        jsonPayload += "\"connected\":true,";
        jsonPayload += "\"ip_address\":\"" + WiFi.localIP().toString() + "\",";
        jsonPayload += "\"mac_address\":\"" + WiFi.macAddress() + "\",";
        jsonPayload += "\"rssi\":" + String(WiFi.RSSI());
    } else {
        jsonPayload += "\"connected\":false";
    }
    jsonPayload += "},";

    // Device Config
    jsonPayload += "\"device\":{";
    jsonPayload += "\"device_id\":\"" + device_id + "\",";
    jsonPayload += "\"send_interval\":" + String(send_interval);
    jsonPayload += "},";

    // System Info
    jsonPayload += "\"system\":{";
    jsonPayload += "\"chip_model\":\"" + String(ESP.getChipModel()) + "\",";
    jsonPayload += "\"chip_revision\":" + String(ESP.getChipRevision()) + ",";
    jsonPayload += "\"cpu_freq_mhz\":" + String(ESP.getCpuFreqMHz()) + ",";
    jsonPayload += "\"free_heap\":" + String(ESP.getFreeHeap()) + ",";
    jsonPayload += "\"flash_size\":" + String(ESP.getFlashChipSize());
    jsonPayload += "},";

    // Timestamp
    jsonPayload += "\"last_updated\":" + String(millis() / 1000);
    jsonPayload += "}";

    Serial.println("Send Config: Sending configuration to Firebase...");
    Serial.print("URL: ");
    Serial.println(url);
    Serial.print("Payload: ");
    Serial.println(jsonPayload);

    // Sử dụng PATCH để cập nhật config, không ghi đè toàn bộ node
    int httpResponseCode = httpConfig.sendRequest("PATCH", jsonPayload);

    if (httpResponseCode > 0) {
        Serial.print("Send Config: HTTP Response code: ");
        Serial.println(httpResponseCode);

        String response = httpConfig.getString();
        Serial.print("Send Config: Response: ");
        Serial.println(response);

        httpConfig.end();

        if (httpResponseCode == 200) {
            Serial.println("Send Config: Configuration sent successfully!");
            return true;
        } else {
            Serial.print("Send Config: Server returned code: ");
            Serial.println(httpResponseCode);
            return false;
        }
    } else {
        Serial.print("Send Config: Error sending config: ");
        Serial.println(httpResponseCode);
        Serial.print("Error: ");
        Serial.println(httpConfig.errorToString(httpResponseCode));
        httpConfig.end();
        return false;
    }
}

// Task chính: gửi config lên Firebase khi WiFi đã kết nối
void taskSendConfigToFirebase(void* pvParameters) {
    Serial.println("Task Send Config to Firebase: Starting...");

    // Đợi cho đến khi WiFi được cấu hình
    while (!wifi_configured) {
        Serial.println("Send Config: Waiting for WiFi configuration...");
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }

    // Đợi cho đến khi WiFi kết nối thành công
    while (WiFi.status() != WL_CONNECTED) {
        Serial.println("Send Config: Waiting for WiFi connection...");
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }

    Serial.println("Send Config: WiFi connected! Sending initial configuration...");

    // Gửi config ngay khi WiFi kết nối lần đầu
    bool success = sendConfigToFirebase();
    if (success) {
        Serial.println("Send Config: Initial configuration sent successfully!");
    } else {
        Serial.println("Send Config: Failed to send initial configuration!");
    }

    // Biến để theo dõi lần gửi cuối cùng và trạng thái WiFi
    unsigned long lastSendTime = 0;
    unsigned long lastWiFiStatus = WL_CONNECTED;
    const unsigned long SEND_INTERVAL = 30000;  // Gửi lại mỗi 30 giây
    bool configSent = false;

    // Vòng lặp chính: gửi lại config khi có thay đổi hoặc định kỳ
    while (1) {
        unsigned long currentTime = millis();
        unsigned long currentWiFiStatus = WiFi.status();

        // Kiểm tra nếu WiFi vừa kết nối lại sau khi mất kết nối
        if (currentWiFiStatus == WL_CONNECTED && lastWiFiStatus != WL_CONNECTED) {
            Serial.println("Send Config: WiFi reconnected! Sending configuration...");
            success = sendConfigToFirebase();
            if (success) {
                Serial.println("Send Config: Configuration sent after reconnection!");
                configSent = true;
                lastSendTime = currentTime;
            }
        }

        // Kiểm tra nếu WiFi đang kết nối và đã đến lúc gửi lại
        if (currentWiFiStatus == WL_CONNECTED) {
            // Gửi lại config nếu:
            // 1. Chưa gửi lần nào
            // 2. Đã qua thời gian interval
            // 3. Hoặc có thay đổi về config (SSID, password, device_id, v.v.)
            bool shouldSend = false;
            if (!configSent) {
                shouldSend = true;
            } else if (currentTime - lastSendTime >= SEND_INTERVAL) {
                shouldSend = true;
            }

            if (shouldSend) {
                success = sendConfigToFirebase();
                if (success) {
                    configSent = true;
                    lastSendTime = currentTime;
                }
            }
        } else {
            Serial.println("Send Config: WiFi disconnected, waiting for reconnection...");
            configSent = false;  // Đánh dấu chưa gửi khi mất kết nối
        }

        lastWiFiStatus = currentWiFiStatus;
        vTaskDelay(5000 / portTICK_PERIOD_MS);  // Kiểm tra mỗi 5 giây
    }
}

