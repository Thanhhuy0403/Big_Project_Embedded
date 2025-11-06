#include "taskWebServer.h"

// Biến global để lưu thông tin WiFi
String wifi_ssid = "";
String wifi_password = "";
bool wifi_configured = false;

WebServer server(80);

// Hàm test kết nối WiFi và trả về kết quả
String testWiFiConnection(String ssid, String password) {
    Serial.println("Testing WiFi connection...");
    Serial.print("SSID: ");
    Serial.println(ssid);

    // Tạm thời disconnect nếu đang kết nối
    WiFi.disconnect();
    vTaskDelay(100 / portTICK_PERIOD_MS);

    // Bắt đầu kết nối
    if (password.length() > 0) {
        WiFi.begin(ssid.c_str(), password.c_str());
    } else {
        WiFi.begin(ssid.c_str());
    }

    // Đợi kết nối với timeout ngắn (10 giây)
    int attempts = 0;
    const int maxAttempts = 20;  // 20 * 500ms = 10 giây

    while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts) {
        vTaskDelay(500 / portTICK_PERIOD_MS);
        attempts++;
    }

    // Kiểm tra kết quả
    wl_status_t status = WiFi.status();
    WiFi.disconnect();
    vTaskDelay(100 / portTICK_PERIOD_MS);

    switch (status) {
        case WL_CONNECTED:
            return "SUCCESS";
        case WL_NO_SSID_AVAIL:
            return "SSID_NOT_FOUND";
        case WL_CONNECT_FAILED:
            return "WRONG_PASSWORD";
        case WL_DISCONNECTED:
        case WL_CONNECTION_LOST:
            return "CONNECTION_FAILED";
        default:
            return "TIMEOUT";
    }
}

// HTML form với UI đẹp hơn và QR code scanner
const char htmlForm[] =
    "<!DOCTYPE html>"
    "<html lang=\"en\">"
    "<head>"
    "<meta charset=\"UTF-8\">"
    "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
    "<title>Yolo Uno WiFi Setup</title>"
    "<style>"
    "*{margin:0;padding:0;box-sizing:border-box}"
    "body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,Oxygen,Ubuntu,Cantarell,sans-serif;"
    "background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);min-height:100vh;display:flex;"
    "justify-content:center;align-items:center;padding:20px}"
    ".container{background:white;border-radius:20px;box-shadow:0 20px 60px rgba(0,0,0,0.3);"
    "max-width:480px;width:100%;overflow:hidden}"
    ".header{background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);color:white;"
    "padding:30px;text-align:center}"
    ".header h1{font-size:28px;margin-bottom:8px}"
    ".header p{font-size:14px;opacity:0.9}"
    ".content{padding:30px}"
    ".form-group{margin-bottom:24px}"
    "label{display:block;margin-bottom:8px;color:#333;font-weight:600;font-size:14px}"
    ".required{color:#e74c3c}"
    ".input-wrapper{position:relative}"
    "input[type=\"text\"],input[type=\"password\"]{width:100%;padding:14px 16px;"
    "border:2px solid #e0e0e0;border-radius:10px;font-size:16px;transition:all 0.3s;background:#f8f9fa}"
    "input[type=\"text\"]:focus,input[type=\"password\"]:focus{outline:none;border-color:#667eea;"
    "background:white;box-shadow:0 0 0 3px rgba(102,126,234,0.1)}"
    "input.error{border-color:#e74c3c;background:#fff5f5}"
    ".error-message{color:#e74c3c;font-size:12px;margin-top:6px;display:none}"
    ".error-message.show{display:block}"
    ".qr-section{margin-bottom:24px;text-align:center}"
    ".qr-button{background:#f8f9fa;border:2px dashed #667eea;border-radius:10px;padding:12px;"
    "cursor:pointer;transition:all 0.3s;color:#667eea;font-weight:600;font-size:14px;width:100%}"
    ".qr-button:hover{background:#667eea;color:white}"
    "#qr-reader{display:none;margin-top:15px}"
    "#qr-reader.show{display:block}"
    "button[type=\"submit\"]{width:100%;padding:16px;"
    "background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);color:white;border:none;"
    "border-radius:10px;font-size:16px;font-weight:600;cursor:pointer;transition:all 0.3s;margin-top:10px}"
    "button[type=\"submit\"]:hover:not(:disabled){transform:translateY(-2px);"
    "box-shadow:0 8px 20px rgba(102,126,234,0.4)}"
    "button[type=\"submit\"]:active:not(:disabled){transform:translateY(0)}"
    "button[type=\"submit\"]:disabled{background:#ccc;cursor:not-allowed;transform:none}"
    ".status{margin-top:20px;padding:14px;border-radius:10px;text-align:center;"
    "font-size:14px;font-weight:500;display:none}"
    ".status.show{display:block}"
    ".status.success{background:#d4edda;color:#155724;border:1px solid #c3e6cb}"
    ".status.error{background:#f8d7da;color:#721c24;border:1px solid #f5c6cb}"
    ".status.warning{background:#fff3cd;color:#856404;border:1px solid #ffeeba}"
    ".status.info{background:#d1ecf1;color:#0c5460;border:1px solid #bee5eb}"
    ".loading{display:inline-block;width:16px;height:16px;border:2px solid #ffffff;"
    "border-radius:50%;border-top-color:transparent;animation:spin 0.8s linear infinite;"
    "margin-right:8px;vertical-align:middle}"
    "@keyframes spin{to{transform:rotate(360deg)}}"
    "</style>"
    "<script src=\"https://unpkg.com/html5-qrcode@latest/html5-qrcode.min.js\"></script>"
    "</head>"
    "<body>"
    "<div class=\"container\">"
    "<div class=\"header\">"
    "<h1>WiFi Setup</h1>"
    "<p>Configure your Yolo Uno connection</p>"
    "</div>"
    "<div class=\"content\">"
    "<form id=\"wifiForm\">"
    "<div class=\"qr-section\">"
    "<button type=\"button\" class=\"qr-button\" id=\"qrButton\" onclick=\"toggleQRScanner()\">"
    "Scan QR Code"
    "</button>"
    "<div id=\"qr-reader\"></div>"
    "</div>"
    "<div class=\"form-group\">"
    "<label for=\"ssid\">WiFi Network Name (SSID) <span class=\"required\">*</span></label>"
    "<div class=\"input-wrapper\">"
    "<input type=\"text\" id=\"ssid\" name=\"ssid\" required placeholder=\"Enter WiFi network name\" "
    "minlength=\"1\" maxlength=\"32\">"
    "</div>"
    "<div class=\"error-message\" id=\"ssidError\">WiFi SSID is required!</div>"
    "</div>"
    "<div class=\"form-group\">"
    "<label for=\"password\">WiFi Password</label>"
    "<div class=\"input-wrapper\">"
    "<input type=\"password\" id=\"password\" name=\"password\" "
    "placeholder=\"Leave empty if no password\" maxlength=\"64\">"
    "</div>"
    "<div class=\"error-message\" id=\"passwordError\"></div>"
    "</div>"
    "<button type=\"submit\" id=\"submitBtn\">Connect to WiFi</button>"
    "</form>"
    "<div id=\"status\" class=\"status\"></div>"
    "</div>"
    "</div>"
    "<script>"
    "const form=document.getElementById('wifiForm');"
    "const ssidInput=document.getElementById('ssid');"
    "const passwordInput=document.getElementById('password');"
    "const ssidError=document.getElementById('ssidError');"
    "const passwordError=document.getElementById('passwordError');"
    "const statusDiv=document.getElementById('status');"
    "const submitBtn=document.getElementById('submitBtn');"
    "const qrReaderDiv=document.getElementById('qr-reader');"
    "let html5QrcodeScanner=null;"
    "let qrScannerActive=false;"
    "function toggleQRScanner(){"
    "if(qrScannerActive)stopQRScanner();else startQRScanner()"
    "}"
    "function startQRScanner(){"
    "if(!html5QrcodeScanner)html5QrcodeScanner=new Html5Qrcode('qr-reader');"
    "html5QrcodeScanner.start({facingMode:'environment'},{fps:10,qrbox:{width:250,height:250}},"
    "onQRCodeSuccess,onQRCodeError);"
    "qrReaderDiv.classList.add('show');"
    "document.getElementById('qrButton').textContent='Stop Scanner';"
    "qrScannerActive=true"
    "}"
    "function stopQRScanner(){"
    "if(html5QrcodeScanner)html5QrcodeScanner.stop().catch(function(err){console.log('Error stopping scanner:',err)});"
    "qrReaderDiv.classList.remove('show');"
    "document.getElementById('qrButton').textContent='Scan QR Code';"
    "qrScannerActive=false"
    "}"
    "function onQRCodeSuccess(decodedText,decodedResult){"
    "console.log('QR Code detected:',decodedText);"
    "if(decodedText.startsWith('WIFI:')){"
    "const params=decodedText.substring(5).split(';');"
    "let ssid='';let password='';"
    "params.forEach(function(param){"
    "if(param.startsWith('S:'))ssid=param.substring(2);"
    "else if(param.startsWith('P:'))password=param.substring(2)"
    "});"
    "if(ssid){"
    "ssidInput.value=ssid;"
    "if(password)passwordInput.value=password;"
    "stopQRScanner();"
    "showStatus('success','QR Code scanned successfully!')"
    "}else showStatus('error','Invalid WiFi QR code format')"
    "}else showStatus('error','Not a WiFi QR code')"
    "}"
    "function onQRCodeError(errorMessage){}"
    "ssidInput.addEventListener('input',function(){validateSSID()});"
    "passwordInput.addEventListener('input',function(){validatePassword()});"
    "function validateSSID(){"
    "const ssid=ssidInput.value.trim();"
    "if(ssid.length===0){"
    "ssidInput.classList.add('error');"
    "ssidError.textContent='WiFi SSID is required!';"
    "ssidError.classList.add('show');"
    "return false"
    "}else if(ssid.length>32){"
    "ssidInput.classList.add('error');"
    "ssidError.textContent='SSID must be 32 characters or less';"
    "ssidError.classList.add('show');"
    "return false"
    "}else{"
    "ssidInput.classList.remove('error');"
    "ssidError.classList.remove('show');"
    "return true"
    "}"
    "}"
    "function validatePassword(){"
    "const password=passwordInput.value;"
    "if(password.length>64){"
    "passwordInput.classList.add('error');"
    "passwordError.textContent='Password must be 64 characters or less';"
    "passwordError.classList.add('show');"
    "return false"
    "}else{"
    "passwordInput.classList.remove('error');"
    "passwordError.classList.remove('show');"
    "return true"
    "}"
    "}"
    "function showStatus(type,message){"
    "statusDiv.className='status '+type+' show';"
    "statusDiv.textContent=message;"
    "if(type==='success')setTimeout(function(){statusDiv.classList.remove('show')},5000)"
    "}"
    "form.addEventListener('submit',async function(e){"
    "e.preventDefault();"
    "if(qrScannerActive)stopQRScanner();"
    "if(!validateSSID()||!validatePassword())return;"
    "const ssid=ssidInput.value.trim();"
    "const password=passwordInput.value;"
    "submitBtn.disabled=true;"
    "submitBtn.innerHTML='<span class=\"loading\"></span>Connecting...';"
    "showStatus('info','Testing WiFi connection...');"
    "const formData=new FormData();"
    "formData.append('ssid',ssid);"
    "formData.append('password',password);"
    "try{"
    "const testResponse=await fetch('/test',{method:'POST',body:formData});"
    "const testResult=await testResponse.text();"
    "if(testResult==='SUCCESS'){"
    "const saveResponse=await fetch('/connect',{method:'POST',body:formData});"
    "const saveResult=await saveResponse.text();"
    "if(saveResponse.ok){"
    "showStatus('success','WiFi configured successfully! Device is connecting...');"
    "submitBtn.innerHTML='✓ Connected!';"
    "submitBtn.style.background='#28a745'"
    "}else{"
    "showStatus('error','Error: '+saveResult);"
    "submitBtn.disabled=false;"
    "submitBtn.textContent='Connect to WiFi'"
    "}"
    "}else{"
    "let errorMsg='';"
    "switch(testResult){"
    "case 'SSID_NOT_FOUND':errorMsg='WiFi network not found. Please check the SSID.';break;"
    "case 'WRONG_PASSWORD':errorMsg='Incorrect password. Please check and try again.';break;"
    "case 'CONNECTION_FAILED':errorMsg='Connection failed. Please try again.';break;"
    "case 'TIMEOUT':errorMsg='Connection timeout. The network may be too far away.';break;"
    "default:errorMsg='Connection failed: '+testResult"
    "}"
    "showStatus('error',errorMsg);"
    "submitBtn.disabled=false;"
    "submitBtn.textContent='Connect to WiFi'"
    "}"
    "}catch(error){"
    "showStatus('error','Network error: '+error.message);"
    "submitBtn.disabled=false;"
    "submitBtn.textContent='Connect to WiFi'"
    "}"
    "});"
    "</script>"
    "</body>"
    "</html>";

// Handler cho trang chủ
void handleRoot() {
    server.send(200, "text/html", htmlForm);
}

// Handler test WiFi connection
void handleTest() {
    if (!server.hasArg("ssid")) {
        server.send(400, "text/plain", "Error: SSID is required");
        return;
    }

    String ssid = server.arg("ssid");
    ssid.trim();

    if (ssid.length() == 0) {
        server.send(400, "text/plain", "Error: SSID cannot be empty");
        return;
    }

    String password = server.arg("password");

    Serial.println("====================");
    Serial.println("Testing WiFi connection from web form:");
    Serial.print("SSID: ");
    Serial.println(ssid);
    Serial.print("Password: ");
    Serial.println(password.length() > 0 ? "***" : "(empty)");

    // Test connection
    String result = testWiFiConnection(ssid, password);

    Serial.print("Test result: ");
    Serial.println(result);
    Serial.println("====================");

    server.send(200, "text/plain", result);
}

// Handler xử lý form submit và lưu cấu hình
void handleConnect() {
    if (!server.hasArg("ssid")) {
        server.send(400, "text/plain", "Error: SSID is required");
        return;
    }

    String ssid = server.arg("ssid");
    ssid.trim();

    if (ssid.length() == 0) {
        server.send(400, "text/plain", "Error: SSID cannot be empty");
        return;
    }

    if (ssid.length() > 32) {
        server.send(400, "text/plain", "Error: SSID must be 32 characters or less");
        return;
    }

    String password = server.arg("password");

    if (password.length() > 64) {
        server.send(400, "text/plain", "Error: Password must be 64 characters or less");
        return;
    }

    // Lưu thông tin WiFi
    wifi_ssid = ssid;
    wifi_password = password;
    wifi_configured = true;

    Serial.println("====================");
    Serial.println("WiFi Configuration saved:");
    Serial.print("SSID: ");
    Serial.println(wifi_ssid);
    Serial.print("Password: ");
    Serial.println(wifi_password.length() > 0 ? "***" : "(empty - open network)");
    Serial.println("====================");

    server.send(200, "text/plain", "WiFi configured successfully! SSID: " + wifi_ssid);
}

void taskWebServer(void* pvParameters) {
    Serial.println("Task Web Server: Starting...");

    // Tạo WiFi Access Point
    Serial.println("Creating WiFi Access Point...");
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(AP_SSID, AP_PASSWORD);

    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);
    Serial.print("AP SSID: ");
    Serial.println(AP_SSID);
    Serial.print("AP Password: ");
    Serial.println(AP_PASSWORD);
    Serial.println("Connect to this WiFi network and open http://192.168.4.1");

    // Cấu hình webserver
    server.on("/", handleRoot);
    server.on("/test", HTTP_POST, handleTest);        // Endpoint để test WiFi
    server.on("/connect", HTTP_POST, handleConnect);  // Endpoint để lưu cấu hình

    server.begin();
    Serial.println("Web server started!");

    // Xử lý requests
    while (1) {
        server.handleClient();
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
