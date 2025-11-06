#include "taskWebServer.h"
String wifi_ssid = "";
String wifi_password = "";
bool wifi_configured = false;

WebServer server(80);
String testWiFiConnection(String ssid, String password) {
    Serial.println("Testing WiFi connection...");
    Serial.print("SSID: ");
    Serial.println(ssid);
    WiFi.disconnect();
    vTaskDelay(100 / portTICK_PERIOD_MS);
    if (password.length() > 0) {
        WiFi.begin(ssid.c_str(), password.c_str());
    } else {
        WiFi.begin(ssid.c_str());
    }
    int attempts = 0;
    const int maxAttempts = 20;

    while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts) {
        vTaskDelay(500 / portTICK_PERIOD_MS);
        attempts++;
    }
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

const char htmlForm[] =
    "<!DOCTYPE html>"
    "<html><head>"
    "<meta charset=\"UTF-8\">"
    "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1.0\">"
    "<title>WiFi Setup</title>"
    "<style>"
    "body{font-family:Arial,sans-serif;background:#667eea;margin:0;padding:20px;display:flex;justify-content:center;align-items:center;min-height:100vh}"
    ".container{background:#fff;border-radius:10px;padding:30px;max-width:400px;width:100%;box-shadow:0 4px 6px rgba(0,0,0,0.1)}"
    "h1{color:#333;margin:0 0 10px 0;font-size:24px;text-align:center}"
    "p{color:#666;text-align:center;margin:0 0 20px 0;font-size:14px}"
    ".form-group{margin-bottom:20px}"
    "label{display:block;color:#333;font-weight:600;margin-bottom:8px;font-size:14px}"
    ".required{color:#e74c3c}"
    "input{width:100%;padding:12px;border:2px solid #ddd;border-radius:5px;font-size:16px;box-sizing:border-box}"
    "input:focus{outline:none;border-color:#667eea}"
    "input.error{border-color:#e74c3c}"
    ".error{color:#e74c3c;font-size:12px;margin-top:5px;display:none}"
    ".error.show{display:block}"
    "button{width:100%;padding:14px;background:#667eea;color:#fff;border:none;border-radius:5px;font-size:16px;font-weight:600;cursor:pointer;margin-top:10px}"
    "button:hover:not(:disabled){background:#5568d3}"
    "button:disabled{background:#ccc;cursor:not-allowed}"
    ".status{margin-top:15px;padding:12px;border-radius:5px;text-align:center;font-size:14px;display:none}"
    ".status.show{display:block}"
    ".status.success{background:#d4edda;color:#155724;border:1px solid #c3e6cb}"
    ".status.error{background:#f8d7da;color:#721c24;border:1px solid #f5c6cb}"
    ".status.info{background:#d1ecf1;color:#0c5460;border:1px solid #bee5eb}"
    "</style>"
    "</head>"
    "<body>"
    "<div class=\"container\">"
    "<h1>WiFi Setup</h1>"
    "<p>Configure your Yolo Uno</p>"
    "<form id=\"wifiForm\">"
    "<div class=\"form-group\">"
    "<label for=\"ssid\">WiFi SSID <span class=\"required\">*</span></label>"
    "<input type=\"text\" id=\"ssid\" name=\"ssid\" required placeholder=\"Enter WiFi name\" maxlength=\"32\">"
    "<div class=\"error\" id=\"ssidError\">SSID is required!</div>"
    "</div>"
    "<div class=\"form-group\">"
    "<label for=\"password\">Password</label>"
    "<input type=\"password\" id=\"password\" name=\"password\" placeholder=\"Leave empty if no password\" maxlength=\"64\">"
    "<div class=\"error\" id=\"passwordError\"></div>"
    "</div>"
    "<button type=\"submit\" id=\"submitBtn\">Connect</button>"
    "</form>"
    "<div class=\"status\" id=\"status\"></div>"
    "</div>"
    "<script>"
    "var f=document.getElementById('wifiForm'),s=document.getElementById('ssid'),p=document.getElementById('password'),"
    "se=document.getElementById('ssidError'),pe=document.getElementById('passwordError'),st=document.getElementById('status'),"
    "sb=document.getElementById('submitBtn');"
    "function vs(){var v=s.value.trim();if(!v){s.classList.add('error');se.textContent='SSID required!';se.classList.add('show');return 0}"
    "if(v.length>32){s.classList.add('error');se.textContent='Max 32 chars';se.classList.add('show');return 0}"
    "s.classList.remove('error');se.classList.remove('show');return 1}"
    "function vp(){var v=p.value;if(v.length>64){p.classList.add('error');pe.textContent='Max 64 chars';pe.classList.add('show');return 0}"
    "p.classList.remove('error');pe.classList.remove('show');return 1}"
    "function ss(t,m){st.className='status '+t+' show';st.textContent=m;if(t=='success')setTimeout(function(){st.classList.remove('show')},5000)}"
    "s.addEventListener('input',vs);p.addEventListener('input',vp);"
    "f.addEventListener('submit',async function(e){e.preventDefault();if(!vs()||!vp())return;"
    "var ssid=s.value.trim(),pass=p.value;sb.disabled=1;sb.textContent='Connecting...';ss('info','Testing connection...');"
    "var fd=new FormData();fd.append('ssid',ssid);fd.append('password',pass);"
    "try{var tr=await fetch('/test',{method:'POST',body:fd}),trt=await tr.text();"
    "if(trt=='SUCCESS'){var sr=await fetch('/connect',{method:'POST',body:fd}),srt=await sr.text();"
    "if(sr.ok){ss('success','WiFi configured! Connecting...');sb.textContent='Connected!';sb.style.background='#28a745'}"
    "else{ss('error','Error: '+srt);sb.disabled=0;sb.textContent='Connect'}}"
    "else{var em='';switch(trt){case 'SSID_NOT_FOUND':em='Network not found';break;"
    "case 'WRONG_PASSWORD':em='Wrong password';break;case 'CONNECTION_FAILED':em='Connection failed';break;"
    "case 'TIMEOUT':em='Timeout';break;default:em='Failed: '+trt}ss('error',em);sb.disabled=0;sb.textContent='Connect'}}"
    "catch(err){ss('error','Error: '+err.message);sb.disabled=0;sb.textContent='Connect'}});"
    "</script>"
    "</body>"
    "</html>";

void handleRoot() {
    server.send(200, "text/html", htmlForm);
}

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
    String result = testWiFiConnection(ssid, password);
    Serial.print("Test result: ");
    Serial.println(result);
    Serial.println("====================");

    server.send(200, "text/plain", result);
}
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
    server.on("/", handleRoot);
    server.on("/test", HTTP_POST, handleTest);
    server.on("/connect", HTTP_POST, handleConnect);

    server.begin();
    Serial.println("Web server started!");
    while (1) {
        server.handleClient();
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
