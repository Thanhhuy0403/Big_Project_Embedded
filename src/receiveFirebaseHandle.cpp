#include "receiveFirebaseHandle.h"

#include "taskWater.h"

HTTPClient httpReceive;
bool glob_remote_pump_enabled = false;
String glob_remote_pump_state = "OFF";
bool glob_remote_led_enabled = false;
String glob_remote_led_state = "ON";

bool fetchPumpControlFromFirebase(String& pumpState) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("ReceiveFirebase: WiFi not connected, cannot fetch data");
        return false;
    }

    String url = "https://" + String(FIREBASE_HOST) + String(FIREBASE_PUMP_CONTROL_PATH) + ".json";

    if (strlen(FIREBASE_AUTH_SECRET) > 0) {
        url += "?auth=" + String(FIREBASE_AUTH_SECRET);
    } else if (strlen(FIREBASE_AUTH_TOKEN) > 0) {
        url += "?auth=" + String(FIREBASE_AUTH_TOKEN);
    }

    httpReceive.begin(url);
    httpReceive.setTimeout(5000);

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
            String originalResponse = response;
            originalResponse.trim();
            if (originalResponse.startsWith("{")) {
                StaticJsonDocument<200> doc;
                DeserializationError error = deserializeJson(doc, originalResponse);

                if (!error) {
                    if (doc.containsKey("state")) {
                        pumpState = doc["state"].as<String>();
                    } else if (doc.containsKey("pump")) {
                        pumpState = doc["pump"].as<String>();
                    } else if (doc.containsKey("status")) {
                        pumpState = doc["status"].as<String>();
                    } else {
                        JsonObject obj = doc.as<JsonObject>();
                        if (obj.size() > 0) {
                            for (JsonPair pair : obj) {
                                pumpState = pair.value().as<String>();
                                break;
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
                pumpState = originalResponse;
                if (pumpState.startsWith("\"")) {
                    pumpState = pumpState.substring(1);
                }
                if (pumpState.endsWith("\"")) {
                    pumpState = pumpState.substring(0, pumpState.length() - 1);
                }
                pumpState.toUpperCase();
            }

            if (pumpState == "ON" || pumpState == "OFF") {
                // Serial.print("ReceiveFirebase: Pump state received: ");
                // Serial.println(pumpState);
                return true;
            } else {
                // Serial.print("ReceiveFirebase: Invalid pump state: ");
                // Serial.println(pumpState);
                return false;
            }
        } else {
            // Serial.print("ReceiveFirebase: Server returned code: ");
            // Serial.println(httpResponseCode);
            return false;
        }
    } else {
        // Serial.print("ReceiveFirebase: Error fetching data: ");
        // Serial.println(httpResponseCode);
        // Serial.print("Error: ");
        // Serial.println(httpReceive.errorToString(httpResponseCode));
        httpReceive.end();
        return false;
    }
}

bool fetchLedControlFromFirebase(String& ledState) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("ReceiveFirebase: WiFi not connected, cannot fetch LED data");
        return false;
    }

    String url = "https://" + String(FIREBASE_HOST) + String(FIREBASE_LED_CONTROL_PATH) + ".json";
    if (strlen(FIREBASE_AUTH_SECRET) > 0) {
        url += "?auth=" + String(FIREBASE_AUTH_SECRET);
    } else if (strlen(FIREBASE_AUTH_TOKEN) > 0) {
        url += "?auth=" + String(FIREBASE_AUTH_TOKEN);
    }
    httpReceive.begin(url);
    httpReceive.setTimeout(5000);
    // Serial.println("ReceiveFirebase: Fetching LED control data...");
    // Serial.print("URL: ");
    // Serial.println(url);

    int httpResponseCode = httpReceive.GET();

    if (httpResponseCode > 0) {
        // Serial.print("ReceiveFirebase: HTTP Response code: ");
        // Serial.println(httpResponseCode);
        String response = httpReceive.getString();
        // Serial.print("ReceiveFirebase: LED Response: ");
        // Serial.println(response);

        httpReceive.end();

        if (httpResponseCode == 200) {
            String originalResponse = response;
            originalResponse.trim();
            if (originalResponse.startsWith("{")) {
                StaticJsonDocument<200> doc;
                DeserializationError error = deserializeJson(doc, originalResponse);

                if (!error) {
                    if (doc.containsKey("state")) {
                        ledState = doc["state"].as<String>();
                    } else if (doc.containsKey("led")) {
                        ledState = doc["led"].as<String>();
                    } else if (doc.containsKey("status")) {
                        ledState = doc["status"].as<String>();
                    } else {
                        JsonObject obj = doc.as<JsonObject>();
                        if (obj.size() > 0) {
                            for (JsonPair pair : obj) {
                                ledState = pair.value().as<String>();
                                break;
                            }
                        }
                    }
                    ledState.toUpperCase();
                } else {
                    // Serial.print("ReceiveFirebase: JSON parse error: ");
                    // Serial.println(error.c_str());
                    return false;
                }
            } else {
                ledState = originalResponse;
                if (ledState.startsWith("\"")) {
                    ledState = ledState.substring(1);
                }
                if (ledState.endsWith("\"")) {
                    ledState = ledState.substring(0, ledState.length() - 1);
                }
                ledState.toUpperCase();
            }

            // Validate LED state
            if (ledState == "ON" || ledState == "OFF") {
                // Serial.print("ReceiveFirebase: LED state received: ");
                // Serial.println(ledState);
                return true;
            } else {
                // Serial.print("ReceiveFirebase: Invalid LED state: ");
                // Serial.println(ledState);
                return false;
            }
        } else {
            // Serial.print("ReceiveFirebase: Server returned code: ");
            // Serial.println(httpResponseCode);
            return false;
        }
    } else {
        // Serial.print("ReceiveFirebase: Error fetching LED data: ");
        // Serial.println(httpResponseCode);
        // Serial.print("Error: ");
        // Serial.println(httpReceive.errorToString(httpResponseCode));
        httpReceive.end();
        return false;
    }
}

// Task chính để nhận và xử lý dữ liệu từ Firebase
void taskReceiveFirebaseHandle(void* pvParameters) {
    Serial.println("Task Receive Firebase Handle: Starting...");
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    const int ledcChannel = 0;

    const int pumpPower = (int)(255 * 0.70);  // 70% công suất
    unsigned long lastFetchTime = 0;
    String lastPumpState = "OFF";
    String lastLedState = "ON";

    while (1) {
        if (WiFi.status() == WL_CONNECTED) {
            unsigned long currentTime = millis();

            if (currentTime - lastFetchTime >= FIREBASE_FETCH_INTERVAL || lastFetchTime == 0) {
                String pumpState;
                bool pumpSuccess = fetchPumpControlFromFirebase(pumpState);

                if (pumpSuccess) {
                    glob_remote_pump_state = pumpState;

                    if (pumpState != lastPumpState) {
                        // Serial.println("====================");
                        // Serial.print("ReceiveFirebase: Pump control state changed: ");
                        // Serial.print(lastPumpState);
                        // Serial.print(" -> ");
                        // Serial.println(pumpState);
                        if (pumpState == "ON") {
                            ledcWrite(ledcChannel, pumpPower);
                            glob_pump_state = true;
                            glob_remote_pump_enabled = true;
                        } else if (pumpState == "OFF") {
                            ledcWrite(ledcChannel, 0);
                            glob_pump_state = false;
                            glob_remote_pump_enabled = true;
                        }
                        lastPumpState = pumpState;
                    }
                } else {
                    Serial.println("ReceiveFirebase: Failed to fetch pump control data");
                }

                // Fetch LED control
                String ledState;
                bool ledSuccess = fetchLedControlFromFirebase(ledState);

                if (ledSuccess) {
                    glob_remote_led_state = ledState;

                    if (ledState != lastLedState) {
                        if (ledState == "ON") {
                            glob_remote_led_enabled = true;
                        } else if (ledState == "OFF") {
                            glob_remote_led_enabled = true;
                        }
                        lastLedState = ledState;
                    }
                } else {
                    Serial.println("ReceiveFirebase: Failed to fetch LED control data");
                }

                lastFetchTime = currentTime;
            }
            vTaskDelay(500 / portTICK_PERIOD_MS);
        } else {
            Serial.println("ReceiveFirebase: Waiting for WiFi connection...");
            vTaskDelay(5000 / portTICK_PERIOD_MS);
        }
    }
}
