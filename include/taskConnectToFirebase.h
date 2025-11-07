#ifndef __TASK_CONNECT_TO_FIREBASE__
#define __TASK_CONNECT_TO_FIREBASE__

#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>

#include "global.h"

// ============================================
// CẤU HÌNH FIREBASE REALTIME DATABASE
// ============================================
// Bước 1: Lấy Firebase Project URL
// - Vào Firebase Console: https://console.firebase.google.com/
// - Chọn project của bạn
// - Vào Realtime Database
// - Copy URL (ví dụ: https://your-project-id-default-rtdb.firebaseio.com/)
// - Bỏ dấu "/" ở cuối nếu có
#define FIREBASE_HOST "db-embedded-74deb-default-rtdb.firebaseio.com/"  // Thay bằng Firebase Host của bạn

// Bước 2: Cấu hình Authentication (chọn 1 trong 2 cách)
// Cách 1: Sử dụng Database Secret (Legacy - dễ dùng hơn)
// - Vào Firebase Console > Project Settings > Service Accounts
// - Copy "Database secret" (nếu có) hoặc tạo mới
// - Hoặc bỏ trống nếu database rules cho phép public
#define FIREBASE_AUTH_SECRET ""  // Thay bằng Database Secret (nếu có), hoặc để trống ""

// Cách 2: Sử dụng Firebase Auth Token (khuyến nghị cho production)
// - Sử dụng Firebase Admin SDK hoặc REST API để lấy token
// - Để trống nếu dùng Database Secret
#define FIREBASE_AUTH_TOKEN ""  // Thay bằng Auth Token (nếu có), hoặc để trống ""

// Bước 3: Cấu hình Database Path
// - Đường dẫn trong Firebase để lưu dữ liệu
// - Ví dụ: "/devices/YoloUno-001" sẽ tạo path: /devices/YoloUno-001/
#define FIREBASE_DB_PATH "/"  // Thay bằng path bạn muốn

// Bước 4: Cấu hình Database Rules (trong Firebase Console)
// Nếu dùng Database Secret, rules có thể là:
// {
//   "rules": {
//     ".read": "auth != null",
//     ".write": "auth != null"
//   }
// }
// Hoặc nếu public (chỉ để test):
// {
//   "rules": {
//     ".read": true,
//     ".write": true
//   }
// }

// Lưu ý: Thời gian gửi dữ liệu (send_interval) được cấu hình thông qua
// webserver và lưu trong Preferences. Sử dụng biến toàn cục glob_send_interval
// từ global.h để truy cập giá trị hiện tại.

void taskConnectToFirebase(void* pvParameters);

#endif
