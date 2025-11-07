#include "receiveFirebaseHandle.h"
#include "taskWater.h"  // Để sử dụng PUMP_PIN_1

HTTPClient httpReceive;

// Định nghĩa các biến toàn cục (đã được khai báo extern trong global.h)
bool glob_remote_pump_enabled = false;  // Bật/tắt điều khiển từ xa
String glob_remote_pump_state = "OFF";  // Trạng thái hiện tại từ Firebase

// Hàm fetch dữ liệu từ Firebase tại path /realtime-pump
bool fetchPumpControlFromFirebase(String& pumpState) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("ReceiveFirebase: WiFi not connected, cannot fetch data");
        return false;
    }

    // Tạo URL để fetch dữ liệu
    String url = "https://" + String(FIREBASE_HOST) + String(FIREBASE_PUMP_CONTROL_PATH) + ".json";
    
    // Thêm authentication nếu có
    if (strlen(FIREBASE_AUTH_SECRET) > 0) {
        url += "?auth=" + String(FIREBASE_AUTH_SECRET);
    } else if (strlen(FIREBASE_AUTH_TOKEN) > 0) {
        url += "?auth=" + String(FIREBASE_AUTH_TOKEN);
    }

    httpReceive.begin(url);
    httpReceive.setTimeout(5000);  // Timeout 5 giây
    
    Serial.println("ReceiveFirebase: Fetching pump control data...");
    Serial.print("URL: ");
    Serial.println(url);

    int httpResponseCode = httpReceive.GET();

    if (httpResponseCode > 0) {
        Serial.print("ReceiveFirebase: HTTP Response code: ");
        Serial.println(httpResponseCode);

        String response = httpReceive.getString();
        Serial.print("ReceiveFirebase: Response: ");
        Serial.println(response);

        httpReceive.end();

        if (httpResponseCode == 200) {
            // Parse JSON response
            // Response có thể là: "ON" hoặc "OFF" (string)
            // Hoặc có thể là object: {"state": "ON"} hoặc {"pump": "OFF"}
            
            String originalResponse = response;  // Lưu response gốc
            originalResponse.trim();
            
            // Kiểm tra nếu là JSON object
            if (originalResponse.startsWith("{")) {
                // Parse JSON object
                StaticJsonDocument<200> doc;
                DeserializationError error = deserializeJson(doc, originalResponse);
                
                if (!error) {
                    // Thử các key có thể có
                    if (doc.containsKey("state")) {
                        pumpState = doc["state"].as<String>();
                    } else if (doc.containsKey("pump")) {
                        pumpState = doc["pump"].as<String>();
                    } else if (doc.containsKey("status")) {
                        pumpState = doc["status"].as<String>();
                    } else {
                        // Nếu không tìm thấy key, lấy giá trị đầu tiên
                        JsonObject obj = doc.as<JsonObject>();
                        if (obj.size() > 0) {
                            // Duyệt qua object để lấy key đầu tiên
                            for (JsonPair pair : obj) {
                                pumpState = pair.value().as<String>();
                                break;  // Chỉ lấy giá trị đầu tiên
                            }
                        }
                    }
                    pumpState.toUpperCase();
                } else {
                    Serial.print("ReceiveFirebase: JSON parse error: ");
                    Serial.println(error.c_str());
                    return false;
                }
            } else {
                // Response là string trực tiếp (có thể có dấu ngoặc kép)
                pumpState = originalResponse;
                // Loại bỏ dấu ngoặc kép nếu có
                if (pumpState.startsWith("\"")) {
                    pumpState = pumpState.substring(1);
                }
                if (pumpState.endsWith("\"")) {
                    pumpState = pumpState.substring(0, pumpState.length() - 1);
                }
                // Chuyển thành chữ hoa để so sánh
                pumpState.toUpperCase();
            }

            // Validate pump state
            if (pumpState == "ON" || pumpState == "OFF") {
                Serial.print("ReceiveFirebase: Pump state received: ");
                Serial.println(pumpState);
                return true;
            } else {
                Serial.print("ReceiveFirebase: Invalid pump state: ");
                Serial.println(pumpState);
                return false;
            }
        } else {
            Serial.print("ReceiveFirebase: Server returned code: ");
            Serial.println(httpResponseCode);
            return false;
        }
    } else {
        Serial.print("ReceiveFirebase: Error fetching data: ");
        Serial.println(httpResponseCode);
        Serial.print("Error: ");
        Serial.println(httpReceive.errorToString(httpResponseCode));
        httpReceive.end();
        return false;
    }
}

// Task chính để nhận và xử lý dữ liệu từ Firebase
void taskReceiveFirebaseHandle(void* pvParameters) {
    Serial.println("Task Receive Firebase Handle: Starting...");
    vTaskDelay(3000 / portTICK_PERIOD_MS);  // Đợi WiFi và Firebase connection sẵn sàng

    // Cấu hình PWM cho máy bơm (sử dụng cùng channel với taskWater)
    // Lưu ý: taskWater đã setup LEDC channel 0 cho PUMP_PIN_1
    // Ở đây ta chỉ cần sử dụng lại channel đó, không cần setup lại
    const int ledcChannel = 0;  // Sử dụng cùng channel với taskWater
    
    const int pumpPower = (int)(255 * 0.70);  // 70% công suất
    unsigned long lastFetchTime = 0;
    String lastPumpState = "OFF";

    while (1) {
        if (WiFi.status() == WL_CONNECTED) {
            unsigned long currentTime = millis();

            // Fetch dữ liệu từ Firebase theo interval
            if (currentTime - lastFetchTime >= FIREBASE_FETCH_INTERVAL || lastFetchTime == 0) {
                String pumpState;
                bool success = fetchPumpControlFromFirebase(pumpState);

                if (success) {
                    glob_remote_pump_state = pumpState;
                    
                    // Chỉ cập nhật nếu trạng thái thay đổi
                    if (pumpState != lastPumpState) {
                        Serial.println("====================");
                        Serial.print("ReceiveFirebase: Pump control state changed: ");
                        Serial.print(lastPumpState);
                        Serial.print(" -> ");
                        Serial.println(pumpState);
                        
                        // Điều khiển máy bơm
                        if (pumpState == "ON") {
                            Serial.println("ReceiveFirebase: Turning pump ON");
                            ledcWrite(ledcChannel, pumpPower);
                            glob_pump_state = true;
                            glob_remote_pump_enabled = true;  // Bật điều khiển từ xa
                        } else if (pumpState == "OFF") {
                            Serial.println("ReceiveFirebase: Turning pump OFF");
                            ledcWrite(ledcChannel, 0);
                            glob_pump_state = false;
                            glob_remote_pump_enabled = true;  // Bật điều khiển từ xa
                        }
                        Serial.println("====================");
                        
                        lastPumpState = pumpState;
                    }
                } else {
                    Serial.println("ReceiveFirebase: Failed to fetch pump control data");
                    // Nếu không fetch được, có thể giữ nguyên trạng thái hiện tại
                    // hoặc tắt điều khiển từ xa để cho phép điều khiển thủ công
                }

                lastFetchTime = currentTime;
            }

            vTaskDelay(500 / portTICK_PERIOD_MS);  // Delay ngắn để không block
        } else {
            Serial.println("ReceiveFirebase: Waiting for WiFi connection...");
            vTaskDelay(5000 / portTICK_PERIOD_MS);
        }
    }
}

