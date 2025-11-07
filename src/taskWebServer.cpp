#include "taskWebServer.h"
#include "taskConnectToFirebase.h"
#include "global.h"
#include <Preferences.h>
#include <LittleFS.h>

// Forward declarations for default values
#ifndef DEFAULT_DEVICE_ID
#define DEFAULT_DEVICE_ID "YoloUno-001"
#endif
#ifndef DEFAULT_SEND_INTERVAL
#define DEFAULT_SEND_INTERVAL 10000
#endif
#ifndef DEFAULT_AP_SSID
#define DEFAULT_AP_SSID "YoloUno-Setup-Thanhhuy"
#endif
#ifndef DEFAULT_AP_PASSWORD
#define DEFAULT_AP_PASSWORD "12345678"
#endif

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

// Helper functions for Preferences (local to this file, no Firebase)
bool loadDeviceConfigFromPreferences(String& device_id, unsigned long& send_interval) {
    Preferences prefs;
    prefs.begin("device_config", true);
    device_id = prefs.getString("device_id", DEFAULT_DEVICE_ID);
    send_interval = prefs.getULong("send_interval", DEFAULT_SEND_INTERVAL);
    prefs.end();
    
    if (device_id.length() > 0 && send_interval > 0) {
        return true;
    }
    return false;
}

bool saveDeviceConfigToPreferences(const String& device_id, unsigned long send_interval) {
    Preferences prefs;
    prefs.begin("device_config", false);
    bool success = prefs.putString("device_id", device_id) && 
                   prefs.putULong("send_interval", send_interval);
    prefs.end();
    return success;
}

bool loadAPConfigFromPreferences(String& ap_ssid, String& ap_password) {
    Preferences prefs;
    prefs.begin("ap_config", true);
    ap_ssid = prefs.getString("ssid", DEFAULT_AP_SSID);
    ap_password = prefs.getString("password", DEFAULT_AP_PASSWORD);
    prefs.end();
    
    if (ap_ssid.length() > 0) {
        return true;
    }
    return false;
}

bool saveAPConfigToPreferences(const String& ap_ssid, const String& ap_password) {
    Preferences prefs;
    prefs.begin("ap_config", false);
    bool success = prefs.putString("ssid", ap_ssid) && prefs.putString("password", ap_password);
    prefs.end();
    return success;
}

String getContentType(String filename) {
    if (filename.endsWith(".html")) return "text/html";
    else if (filename.endsWith(".css")) return "text/css";
    else if (filename.endsWith(".js")) return "application/javascript";
    else if (filename.endsWith(".png")) return "image/png";
    else if (filename.endsWith(".jpg")) return "image/jpeg";
    else if (filename.endsWith(".gif")) return "image/gif";
    else if (filename.endsWith(".ico")) return "image/x-icon";
    else if (filename.endsWith(".json")) return "application/json";
    return "text/plain";
}

bool handleFileRead(String path) {
    Serial.println("handleFileRead: " + path);
    if (path.endsWith("/")) {
        path += "index.html";
    }
    String contentType = getContentType(path);
    String pathWithGz = path + ".gz";
    if (LittleFS.exists(pathWithGz) || LittleFS.exists(path)) {
        if (LittleFS.exists(pathWithGz)) {
            path += ".gz";
        }
        File file = LittleFS.open(path, "r");
        if (server.streamFile(file, contentType) != file.size()) {
            Serial.println("Sent less data than expected!");
        }
        file.close();
        return true;
    }
    Serial.println("\tFile Not Found");
    return false;
}

void handleRoot() {
    // Try to serve from LittleFS first
    if (handleFileRead("/index.html")) {
        return;
    }
    
    // Fallback: redirect to inline HTML page
    server.sendHeader("Location", "/inline", true);
    server.send(302, "text/plain", "");
}

void handleInline() {
    // Serve inline HTML with embedded CSS and JS
    server.send(200, "text/html", 
        "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width,initial-scale=1.0\"><title>ESP32 Configuration</title>"
        "<style>body{font-family:Arial,sans-serif;background:#667eea;margin:0;padding:20px;min-height:100vh}.container{background:#fff;border-radius:10px;padding:30px;max-width:600px;width:100%;box-shadow:0 4px 6px rgba(0,0,0,0.1);margin:0 auto}h1{color:#333;margin:0 0 20px 0;font-size:24px;text-align:center}h2{color:#333;margin:0 0 20px 0;font-size:20px}.tabs{display:flex;border-bottom:2px solid #ddd;margin-bottom:20px}.tab{flex:1;padding:12px;text-align:center;cursor:pointer;border:none;background:#f5f5f5;color:#666;font-weight:600;transition:all 0.3s}.tab:hover{background:#e0e0e0}.tab.active{background:#667eea;color:#fff}.tab-content{display:none}.tab-content.active{display:block}.form-group{margin-bottom:20px}label{display:block;color:#333;font-weight:600;margin-bottom:8px;font-size:14px}.required{color:#e74c3c}input{width:100%;padding:12px;border:2px solid #ddd;border-radius:5px;font-size:16px;box-sizing:border-box}input:focus{outline:none;border-color:#667eea}input.error{border-color:#e74c3c}.error{color:#e74c3c;font-size:12px;margin-top:5px;display:none}.error.show{display:block}button{width:100%;padding:14px;background:#667eea;color:#fff;border:none;border-radius:5px;font-size:16px;font-weight:600;cursor:pointer;margin-top:10px;transition:background 0.3s}button:hover:not(:disabled){background:#5568d3}button:disabled{background:#ccc;cursor:not-allowed}.status{margin-top:15px;padding:12px;border-radius:5px;text-align:center;font-size:14px;display:none}.status.show{display:block}.status.success{background:#d4edda;color:#155724;border:1px solid #c3e6cb}.status.error{background:#f8d7da;color:#721c24;border:1px solid #f5c6cb}.status.info{background:#d1ecf1;color:#0c5460;border:1px solid #bee5eb}</style>"
        "</head><body><div class=\"container\"><h1>ESP32 Configuration</h1><div class=\"tabs\"><button class=\"tab active\" onclick=\"showTab(0)\">WiFi</button><button class=\"tab\" onclick=\"showTab(1)\">Device</button><button class=\"tab\" onclick=\"showTab(2)\">AP Config</button></div>"
        "<div id=\"tab0\" class=\"tab-content active\"><h2>WiFi Configuration</h2><form id=\"wifiForm\"><div class=\"form-group\"><label for=\"wifi_ssid\">WiFi SSID <span class=\"required\">*</span></label><input type=\"text\" id=\"wifi_ssid\" required placeholder=\"Enter WiFi name\" maxlength=\"32\"><div class=\"error\" id=\"wifi_ssidError\"></div></div><div class=\"form-group\"><label for=\"wifi_password\">Password</label><input type=\"password\" id=\"wifi_password\" placeholder=\"Leave empty if no password\" maxlength=\"64\"><div class=\"error\" id=\"wifi_passwordError\"></div></div><button type=\"submit\" id=\"wifiBtn\">Connect WiFi</button></form></div>"
        "<div id=\"tab1\" class=\"tab-content\"><h2>Device Configuration</h2><form id=\"deviceForm\"><div class=\"form-group\"><label for=\"device_id\">Device ID <span class=\"required\">*</span></label><input type=\"text\" id=\"device_id\" required placeholder=\"Enter device ID\" maxlength=\"32\"><div class=\"error\" id=\"device_idError\"></div></div><div class=\"form-group\"><label for=\"send_interval\">Send Interval (ms) <span class=\"required\">*</span></label><input type=\"number\" id=\"send_interval\" required placeholder=\"10000\" min=\"1000\" max=\"600000\"><div class=\"error\" id=\"send_intervalError\"></div></div><button type=\"submit\" id=\"deviceBtn\">Save Device Config</button></form></div>"
        "<div id=\"tab2\" class=\"tab-content\"><h2>AP Configuration</h2><form id=\"apForm\"><div class=\"form-group\"><label for=\"ap_ssid\">AP SSID <span class=\"required\">*</span></label><input type=\"text\" id=\"ap_ssid\" required placeholder=\"Enter AP name\" maxlength=\"32\"><div class=\"error\" id=\"ap_ssidError\"></div></div><div class=\"form-group\"><label for=\"ap_password\">AP Password</label><input type=\"password\" id=\"ap_password\" placeholder=\"Enter AP password\" maxlength=\"64\"><div class=\"error\" id=\"ap_passwordError\"></div></div><button type=\"submit\" id=\"apBtn\">Save AP Config</button></form></div>"
        "<div class=\"status\" id=\"status\"></div></div>"
        "<script>function showTab(n){var tabs=document.querySelectorAll('.tab');var contents=document.querySelectorAll('.tab-content');for(var i=0;i<tabs.length;i++){tabs[i].classList.remove('active');contents[i].classList.remove('active')}tabs[n].classList.add('active');contents[n].classList.add('active')}function showStatus(t,m){var st=document.getElementById('status');st.className='status '+t+' show';st.textContent=m;if(t=='success')setTimeout(function(){st.classList.remove('show')},5000)}function validateWifi(){var s=document.getElementById('wifi_ssid'),p=document.getElementById('wifi_password'),se=document.getElementById('wifi_ssidError'),pe=document.getElementById('wifi_passwordError');var v=s.value.trim();if(!v){s.classList.add('error');se.textContent='SSID required!';se.classList.add('show');return false}if(v.length>32){s.classList.add('error');se.textContent='Max 32 chars';se.classList.add('show');return false}s.classList.remove('error');se.classList.remove('show');if(p.value.length>64){p.classList.add('error');pe.textContent='Max 64 chars';pe.classList.add('show');return false}p.classList.remove('error');pe.classList.remove('show');return true}function validateDevice(){var d=document.getElementById('device_id'),i=document.getElementById('send_interval'),de=document.getElementById('device_idError'),ie=document.getElementById('send_intervalError');var v=d.value.trim();if(!v){d.classList.add('error');de.textContent='Device ID required!';de.classList.add('show');return false}if(v.length>32){d.classList.add('error');de.textContent='Max 32 chars';de.classList.add('show');return false}d.classList.remove('error');de.classList.remove('show');var iv=parseInt(i.value);if(isNaN(iv)||iv<1000||iv>600000){i.classList.add('error');ie.textContent='Interval must be 1000-600000 ms';ie.classList.add('show');return false}i.classList.remove('error');ie.classList.remove('show');return true}function validateAP(){var s=document.getElementById('ap_ssid'),p=document.getElementById('ap_password'),se=document.getElementById('ap_ssidError'),pe=document.getElementById('ap_passwordError');var v=s.value.trim();if(!v){s.classList.add('error');se.textContent='AP SSID required!';se.classList.add('show');return false}if(v.length>32){s.classList.add('error');se.textContent='Max 32 chars';se.classList.add('show');return false}s.classList.remove('error');se.classList.remove('show');if(p.value.length>64){p.classList.add('error');pe.textContent='Max 64 chars';pe.classList.add('show');return false}p.classList.remove('error');pe.classList.remove('show');return true}document.getElementById('wifiForm').addEventListener('submit',async function(e){e.preventDefault();if(!validateWifi())return;var btn=document.getElementById('wifiBtn');btn.disabled=true;btn.textContent='Testing...';showStatus('info','Testing connection...');var fd=new FormData();fd.append('ssid',document.getElementById('wifi_ssid').value.trim());fd.append('password',document.getElementById('wifi_password').value);try{var tr=await fetch('/test',{method:'POST',body:fd}),trt=await tr.text();if(trt=='SUCCESS'){var sr=await fetch('/connect',{method:'POST',body:fd}),srt=await sr.text();if(sr.ok){showStatus('success','WiFi configured!');btn.textContent='Connected!';btn.style.background='#28a745'}else{showStatus('error','Error: '+srt);btn.disabled=false;btn.textContent='Connect WiFi'}}else{var em='';switch(trt){case 'SSID_NOT_FOUND':em='Network not found';break;case 'WRONG_PASSWORD':em='Wrong password';break;case 'CONNECTION_FAILED':em='Connection failed';break;case 'TIMEOUT':em='Timeout';break;default:em='Failed: '+trt}showStatus('error',em);btn.disabled=false;btn.textContent='Connect WiFi'}}catch(err){showStatus('error','Error: '+err.message);btn.disabled=false;btn.textContent='Connect WiFi'}});document.getElementById('deviceForm').addEventListener('submit',async function(e){e.preventDefault();if(!validateDevice())return;var btn=document.getElementById('deviceBtn');btn.disabled=true;btn.textContent='Saving...';showStatus('info','Saving device config...');var fd=new FormData();fd.append('device_id',document.getElementById('device_id').value.trim());fd.append('send_interval',document.getElementById('send_interval').value);try{var r=await fetch('/device',{method:'POST',body:fd}),rt=await r.text();if(r.ok){showStatus('success','Device config saved!');btn.textContent='Saved!';setTimeout(function(){btn.disabled=false;btn.textContent='Save Device Config'},2000)}else{showStatus('error','Error: '+rt);btn.disabled=false;btn.textContent='Save Device Config'}}catch(err){showStatus('error','Error: '+err.message);btn.disabled=false;btn.textContent='Save Device Config'}});document.getElementById('apForm').addEventListener('submit',async function(e){e.preventDefault();if(!validateAP())return;var btn=document.getElementById('apBtn');btn.disabled=true;btn.textContent='Saving...';showStatus('info','Saving AP config...');var fd=new FormData();fd.append('ap_ssid',document.getElementById('ap_ssid').value.trim());fd.append('ap_password',document.getElementById('ap_password').value);try{var r=await fetch('/ap',{method:'POST',body:fd}),rt=await r.text();if(r.ok){showStatus('success','AP config saved! Restart required.');btn.textContent='Saved!';setTimeout(function(){btn.disabled=false;btn.textContent='Save AP Config'},2000)}else{showStatus('error','Error: '+rt);btn.disabled=false;btn.textContent='Save AP Config'}}catch(err){showStatus('error','Error: '+err.message);btn.disabled=false;btn.textContent='Save AP Config'}});window.addEventListener('load',async function(){try{var r=await fetch('/config');if(r.ok){var d=await r.json();if(d.device_id)document.getElementById('device_id').value=d.device_id;if(d.send_interval)document.getElementById('send_interval').value=d.send_interval;if(d.ap_ssid)document.getElementById('ap_ssid').value=d.ap_ssid;if(d.ap_password)document.getElementById('ap_password').value=d.ap_password}}catch(err){console.log('Failed to load config:',err)}});</script></body></html>"
    );
}

void handleConfig() {
    String device_id;
    unsigned long send_interval;
    loadDeviceConfigFromPreferences(device_id, send_interval);
    
    String ap_ssid, ap_password;
    loadAPConfigFromPreferences(ap_ssid, ap_password);
    
    String json = "{";
    json += "\"device_id\":\"" + device_id + "\",";
    json += "\"send_interval\":" + String(send_interval) + ",";
    json += "\"ap_ssid\":\"" + ap_ssid + "\",";
    json += "\"ap_password\":\"" + ap_password + "\"";
    json += "}";
    
    server.send(200, "application/json", json);
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

void handleDevice() {
    if (!server.hasArg("device_id") || !server.hasArg("send_interval")) {
        server.send(400, "text/plain", "Error: device_id and send_interval are required");
        return;
    }
    
    String device_id = server.arg("device_id");
    device_id.trim();
    if (device_id.length() == 0 || device_id.length() > 32) {
        server.send(400, "text/plain", "Error: device_id must be 1-32 characters");
        return;
    }
    
    unsigned long send_interval = server.arg("send_interval").toInt();
    if (send_interval < 1000 || send_interval > 600000) {
        server.send(400, "text/plain", "Error: send_interval must be 1000-600000 ms");
        return;
    }
    
    saveDeviceConfigToPreferences(device_id, send_interval);
    glob_device_id = device_id;
    
    Serial.println("====================");
    Serial.println("Device Configuration saved:");
    Serial.print("Device ID: ");
    Serial.println(device_id);
    Serial.print("Send Interval: ");
    Serial.print(send_interval);
    Serial.println(" ms");
    Serial.println("====================");
    
    server.send(200, "text/plain", "Device config saved successfully!");
}

void handleAP() {
    if (!server.hasArg("ap_ssid")) {
        server.send(400, "text/plain", "Error: ap_ssid is required");
        return;
    }
    
    String ap_ssid = server.arg("ap_ssid");
    ap_ssid.trim();
    if (ap_ssid.length() == 0 || ap_ssid.length() > 32) {
        server.send(400, "text/plain", "Error: ap_ssid must be 1-32 characters");
        return;
    }
    
    String ap_password = server.arg("ap_password");
    if (ap_password.length() > 64) {
        server.send(400, "text/plain", "Error: ap_password must be 64 characters or less");
        return;
    }
    
    saveAPConfigToPreferences(ap_ssid, ap_password);
    
    Serial.println("====================");
    Serial.println("AP Configuration saved:");
    Serial.print("AP SSID: ");
    Serial.println(ap_ssid);
    Serial.print("AP Password: ");
    Serial.println(ap_password.length() > 0 ? "***" : "(empty)");
    Serial.println("Note: ESP32 needs to restart to apply new AP config");
    Serial.println("====================");
    
    server.send(200, "text/plain", "AP config saved successfully! Restart required.");
}

void taskWebServer(void* pvParameters) {
    Serial.println("Task Web Server: Starting...");
    
    // Initialize LittleFS
    bool fsMounted = LittleFS.begin(true);
    if (fsMounted) {
        Serial.println("LittleFS mounted successfully");
        
        // List files in LittleFS
        File root = LittleFS.open("/");
        Serial.println("Files in LittleFS:");
        File file = root.openNextFile();
        bool hasFiles = false;
        while (file) {
            hasFiles = true;
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());
            file = root.openNextFile();
        }
        if (!hasFiles) {
            Serial.println("  (No files found)");
        }
    } else {
        Serial.println("LittleFS Mount Failed - will use inline HTML fallback");
        Serial.println("To use files from LittleFS, upload filesystem image:");
        Serial.println("  pio run --target uploadfs");
    }
    
    // Load AP config from Preferences
    String ap_ssid, ap_password;
    Preferences prefs;
    prefs.begin("ap_config", true);
    ap_ssid = prefs.getString("ssid", DEFAULT_AP_SSID);
    ap_password = prefs.getString("password", DEFAULT_AP_PASSWORD);
    prefs.end();
    
    Serial.println("Creating WiFi Access Point...");
    Serial.print("AP SSID: ");
    Serial.println(ap_ssid);
    Serial.print("AP Password: ");
    Serial.println(ap_password);
    
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(ap_ssid.c_str(), ap_password.c_str());

    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);
    Serial.println("Connect to this WiFi network and open http://192.168.4.1");
    
    // Serve static files from LittleFS
    server.onNotFound([]() {
        if (!handleFileRead(server.uri())) {
            server.send(404, "text/plain", "FileNotFound");
        }
    });
    
    // API endpoints
    server.on("/", handleRoot);
    server.on("/inline", handleInline);
    server.on("/config", handleConfig);
    server.on("/test", HTTP_POST, handleTest);
    server.on("/connect", HTTP_POST, handleConnect);
    server.on("/device", HTTP_POST, handleDevice);
    server.on("/ap", HTTP_POST, handleAP);

    server.begin();
    Serial.println("Web server started!");
    
    while (1) {
        server.handleClient();
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
