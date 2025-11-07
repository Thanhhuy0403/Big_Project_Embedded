function showTab(n) {
    var tabs = document.querySelectorAll('.tab');
    var contents = document.querySelectorAll('.tab-content');
    
    // Remove active class with smooth transition
    for (var i = 0; i < tabs.length; i++) {
        tabs[i].classList.remove('active');
        if (contents[i].classList.contains('active')) {
            contents[i].style.opacity = '0';
            setTimeout(function(idx) {
                return function() {
                    contents[idx].classList.remove('active');
                };
            }(i), 150);
        }
    }
    
    // Add active class with smooth transition
    setTimeout(function() {
        tabs[n].classList.add('active');
        contents[n].classList.add('active');
        contents[n].style.opacity = '0';
        setTimeout(function() {
            contents[n].style.opacity = '1';
        }, 10);
    }, 150);
}

function togglePassword(inputId, button) {
    var input = document.getElementById(inputId);
    var eyeOpen = button.querySelector('.eye-open');
    var eyeClosed = button.querySelector('.eye-closed');
    
    // Toggle input type
    if (input.type === 'password') {
        input.type = 'text';
        button.setAttribute('aria-label', 'Hide password');
    } else {
        input.type = 'password';
        button.setAttribute('aria-label', 'Show password');
    }
    
    // CSS sẽ tự động xử lý opacity dựa trên input type
    // Không cần set style trực tiếp vì CSS đã có rules
}

function showStatus(t, m) {
    var st = document.getElementById('status');
    st.className = 'status ' + t + ' show';
    st.textContent = m;
    
    // Auto-hide success messages with smooth fade
    if (t == 'success') {
        setTimeout(function() {
            st.style.opacity = '0';
            st.style.transform = 'translateY(-10px) scale(0.95)';
            setTimeout(function() {
                st.classList.remove('show');
                st.style.opacity = '';
                st.style.transform = '';
            }, 300);
        }, 4000);
    }
}

function validateWifi() {
    var s = document.getElementById('wifi_ssid');
    var p = document.getElementById('wifi_password');
    var se = document.getElementById('wifi_ssidError');
    var pe = document.getElementById('wifi_passwordError');
    
    var v = s.value.trim();
    if (!v) {
        s.classList.add('error');
        se.textContent = 'SSID required!';
        se.classList.add('show');
        return false;
    }
    if (v.length > 32) {
        s.classList.add('error');
        se.textContent = 'Max 32 chars';
        se.classList.add('show');
        return false;
    }
    s.classList.remove('error');
    se.classList.remove('show');
    
    if (p.value.length > 64) {
        p.classList.add('error');
        pe.textContent = 'Max 64 chars';
        pe.classList.add('show');
        return false;
    }
    p.classList.remove('error');
    pe.classList.remove('show');
    
    return true;
}

function validateDevice() {
    var d = document.getElementById('device_id');
    var i = document.getElementById('send_interval');
    var de = document.getElementById('device_idError');
    var ie = document.getElementById('send_intervalError');
    
    var v = d.value.trim();
    if (!v) {
        d.classList.add('error');
        de.textContent = 'Device ID required!';
        de.classList.add('show');
        return false;
    }
    if (v.length > 32) {
        d.classList.add('error');
        de.textContent = 'Max 32 chars';
        de.classList.add('show');
        return false;
    }
    d.classList.remove('error');
    de.classList.remove('show');
    
    var iv = parseInt(i.value);
    if (isNaN(iv) || iv < 1000 || iv > 600000) {
        i.classList.add('error');
        ie.textContent = 'Interval must be 1000-600000 ms';
        ie.classList.add('show');
        return false;
    }
    i.classList.remove('error');
    ie.classList.remove('show');
    
    return true;
}

function validateAP() {
    var s = document.getElementById('ap_ssid');
    var p = document.getElementById('ap_password');
    var i = document.getElementById('ap_send_interval');
    var se = document.getElementById('ap_ssidError');
    var pe = document.getElementById('ap_passwordError');
    var ie = document.getElementById('ap_send_intervalError');
    
    var v = s.value.trim();
    if (!v) {
        s.classList.add('error');
        se.textContent = 'AP SSID required!';
        se.classList.add('show');
        return false;
    }
    if (v.length > 32) {
        s.classList.add('error');
        se.textContent = 'Max 32 chars';
        se.classList.add('show');
        return false;
    }
    s.classList.remove('error');
    se.classList.remove('show');
    
    if (p.value.length > 64) {
        p.classList.add('error');
        pe.textContent = 'Max 64 chars';
        pe.classList.add('show');
        return false;
    }
    p.classList.remove('error');
    pe.classList.remove('show');
    
    var iv = parseInt(i.value);
    if (isNaN(iv) || iv < 1000 || iv > 600000) {
        i.classList.add('error');
        ie.textContent = 'Interval must be 1000-600000 ms';
        ie.classList.add('show');
        return false;
    }
    i.classList.remove('error');
    ie.classList.remove('show');
    
    return true;
}

// WiFi Form Handler
document.getElementById('wifiForm').addEventListener('submit', async function(e) {
    e.preventDefault();
    if (!validateWifi()) return;
    
    var btn = document.getElementById('wifiBtn');
    var originalText = btn.textContent;
    btn.disabled = true;
    btn.innerHTML = '<span class="loading"></span>Testing...';
    showStatus('info', 'Testing connection...');
    
    var fd = new FormData();
    fd.append('ssid', document.getElementById('wifi_ssid').value.trim());
    fd.append('password', document.getElementById('wifi_password').value);
    
    try {
        var tr = await fetch('/test', { method: 'POST', body: fd });
        var trt = await tr.text();
        
        if (trt == 'SUCCESS') {
            var sr = await fetch('/connect', { method: 'POST', body: fd });
            var srt = await sr.text();
            
            if (sr.ok) {
                showStatus('success', 'WiFi configured!');
                btn.textContent = 'Connected!';
                btn.style.background = '#28a745';
            } else {
                showStatus('error', 'Error: ' + srt);
                btn.disabled = false;
                btn.textContent = originalText;
                btn.style.background = '';
            }
        } else {
            var em = '';
            switch (trt) {
                case 'SSID_NOT_FOUND':
                    em = 'Network not found';
                    break;
                case 'WRONG_PASSWORD':
                    em = 'Wrong password';
                    break;
                case 'CONNECTION_FAILED':
                    em = 'Connection failed';
                    break;
                case 'TIMEOUT':
                    em = 'Timeout';
                    break;
                default:
                    em = 'Failed: ' + trt;
            }
            showStatus('error', em);
            btn.disabled = false;
            btn.textContent = originalText;
            btn.style.background = '';
        }
    } catch (err) {
        showStatus('error', 'Error: ' + err.message);
        btn.disabled = false;
        btn.textContent = originalText;
        btn.style.background = '';
    }
});

// Device Form Handler
document.getElementById('deviceForm').addEventListener('submit', async function(e) {
    e.preventDefault();
    if (!validateDevice()) return;
    
    var btn = document.getElementById('deviceBtn');
    var originalText = btn.textContent;
    btn.disabled = true;
    btn.innerHTML = '<span class="loading"></span>Saving...';
    showStatus('info', 'Saving device config...');
    
    var fd = new FormData();
    fd.append('device_id', document.getElementById('device_id').value.trim());
    fd.append('send_interval', document.getElementById('send_interval').value);
    
    try {
        var r = await fetch('/device', { method: 'POST', body: fd });
        var rt = await r.text();
        
        if (r.ok) {
            showStatus('success', 'Device config saved!');
            btn.textContent = '✓ Saved!';
            btn.style.background = 'linear-gradient(135deg, #10b981 0%, #059669 100%)';
            setTimeout(function() {
                btn.disabled = false;
                btn.textContent = originalText;
                btn.style.background = '';
            }, 2000);
        } else {
            showStatus('error', 'Error: ' + rt);
            btn.disabled = false;
            btn.textContent = originalText;
        }
    } catch (err) {
        showStatus('error', 'Error: ' + err.message);
        btn.disabled = false;
        btn.textContent = originalText;
    }
});

// AP Form Handler
document.getElementById('apForm').addEventListener('submit', async function(e) {
    e.preventDefault();
    if (!validateAP()) return;
    
    var btn = document.getElementById('apBtn');
    var originalText = btn.textContent;
    btn.disabled = true;
    btn.innerHTML = '<span class="loading"></span>Saving...';
    showStatus('info', 'Saving AP config...');
    
    var fd = new FormData();
    fd.append('ap_ssid', document.getElementById('ap_ssid').value.trim());
    fd.append('ap_password', document.getElementById('ap_password').value);
    fd.append('send_interval', document.getElementById('ap_send_interval').value);
    
    try {
        var r = await fetch('/ap', { method: 'POST', body: fd });
        var rt = await r.text();
        
        if (r.ok) {
            showStatus('success', 'AP config saved! Restart required.');
            btn.textContent = '✓ Saved!';
            btn.style.background = 'linear-gradient(135deg, #10b981 0%, #059669 100%)';
            setTimeout(function() {
                btn.disabled = false;
                btn.textContent = originalText;
                btn.style.background = '';
            }, 2000);
        } else {
            showStatus('error', 'Error: ' + rt);
            btn.disabled = false;
            btn.textContent = originalText;
        }
    } catch (err) {
        showStatus('error', 'Error: ' + err.message);
        btn.disabled = false;
        btn.textContent = originalText;
    }
});

// Load config on page load
window.addEventListener('load', async function() {
    try {
        var r = await fetch('/config');
        if (r.ok) {
            var d = await r.json();
            if (d.device_id) document.getElementById('device_id').value = d.device_id;
            if (d.send_interval) {
                document.getElementById('send_interval').value = d.send_interval;
                document.getElementById('ap_send_interval').value = d.send_interval;
            }
            if (d.ap_ssid) document.getElementById('ap_ssid').value = d.ap_ssid;
            if (d.ap_password) document.getElementById('ap_password').value = d.ap_password;
        }
    } catch (err) {
        console.log('Failed to load config:', err);
    }
});

