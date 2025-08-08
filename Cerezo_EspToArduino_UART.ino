#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>

// === WiFi Settings ===
const char* WIFI_SSID = "PLDTHOMEFIB3R";
const char* WIFI_PASS = "PLDTWIFIfib3r";

// === Firebase Configuration ===
// 🔥 IMPORTANT: Must have public .write: true rules
const String FIREBASE_HOST = "https://integrating-uart-default-rtdb.firebaseio.com/";
const String FIREBASE_PATH = "/sensorData.json";

// === Web Server ===
WebServer server(80);

// === Sensor Data Storage ===
struct SensorData {
  String dhtTemp = "N/A";
  String dhtHum = "N/A";
  String lm35Temp = "N/A";
  String ldrValue = "N/A";
  String waterStatus = "N/A";
} sensorData;

// === Function Prototypes ===
String parseValue(String data, String key);
void updateFirebase();
void handleRoot();
void handleApiData();
String addQuotes(String value);

// === Helper: Add quotes to strings in JSON ===
String addQuotes(String value) {
  if (value == "N/A" || value == "Dry" || value == "Wet") {
    return "\"" + value + "\"";
  }
  return value; // Numbers stay unquoted
}

// === Parse value from T1:25.0|H:60|T2:26.5|L:420|W:Dry ===
String parseValue(String data, String key) {
  int start = data.indexOf(key + ":");
  if (start == -1) return "N/A";
  start += key.length() + 1;
  int end = data.indexOf('|', start);
  if (end == -1) end = data.length();
  return data.substring(start, end);
}

// === Send data to Firebase ===
void updateFirebase() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[Firebase] ❌ WiFi disconnected");
    return;
  }

  HTTPClient http;
  String url = FIREBASE_HOST + FIREBASE_PATH;
  http.begin(url);
  http.addHeader("Content-Type", "application/json");

  String payload = "{";
  payload += "\"dhtTemp\":" + addQuotes(sensorData.dhtTemp) + ",";
  payload += "\"dhtHum\":" + addQuotes(sensorData.dhtHum) + ",";
  payload += "\"lm35Temp\":" + addQuotes(sensorData.lm35Temp) + ",";
  payload += "\"ldr\":\"" + sensorData.ldrValue + "\",";  // Always quoted
  payload += "\"water\":" + addQuotes(sensorData.waterStatus);
  payload += "}";

  Serial.println("[Firebase] > Sending data...");
  Serial.println("URL: " + url);
  Serial.println("Payload: " + payload);

  int httpCode = http.PUT(payload);
  String response = http.getString();
  Serial.print("[Firebase] HTTP: ");
  Serial.println(httpCode);
  Serial.println("[Firebase] Response: ");
  Serial.println(response);  // If this shows HTML login → Firebase rules issue

  if (httpCode == 200) {
    Serial.println("✅ Success: Data updated in Firebase!");
  } else {
    Serial.println("❌ Failed to send data. Check URL and rules.");
  }

  http.end();
}

// === Web Dashboard ===
void handleRoot() {
  String html = R"html(
<!DOCTYPE html>
<html>
<head>
  <title>Sensor Dashboard</title>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body {
      font-family: 'Segoe UI', sans-serif;
      background: #f5f8fa;
      text-align: center;
      padding: 20px;
    }
    h1 {
      color: #2980b9;
      margin-bottom: 20px;
    }
    .container {
      display: flex;
      flex-wrap: wrap;
      justify-content: center;
      gap: 15px;
      max-width: 900px;
      margin: 0 auto;
    }
    .card {
      background: white;
      border-radius: 12px;
      box-shadow: 0 4px 8px rgba(0,0,0,0.1);
      width: 160px;
      padding: 16px;
    }
    .icon {
      font-size: 24px;
      color: #3498db;
    }
    .label {
      font-size: 14px;
      color: #7f8c8d;
    }
    .value {
      font-size: 18px;
      font-weight: bold;
      color: #2c3e50;
    }
    .status-dry { color: #e74c3c; }
    .status-wet { color: #27ae60; }
    footer {
      margin-top: 40px;
      color: #95a5a6;
      font-size: 0.9em;
    }
  </style>
  <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.5.0/css/all.min.css">
</head>
<body>
  <h1>🌿 Sensor Hub</h1>
  <div class="container">
    <div class="card">
      <div class="icon"><i class="fas fa-thermometer-half"></i></div>
      <div class="label">DHT Temp</div>
      <div class="value" id="dhtTemp">--</div>
    </div>
    <div class="card">
      <div class="icon"><i class="fas fa-tint"></i></div>
      <div class="label">Humidity</div>
      <div class="value" id="dhtHum">--</div>
    </div>
    <div class="card">
      <div class="icon"><i class="fas fa-fire"></i></div>
      <div class="label">LM35 Temp</div>
      <div class="value" id="lm35">--</div>
    </div>
    <div class="card">
      <div class="icon"><i class="fas fa-lightbulb"></i></div>
      <div class="label">Light</div>
      <div class="value" id="ldr">--</div>
    </div>
    <div class="card">
      <div class="icon"><i class="fas fa-faucet"></i></div>
      <div class="label">Water</div>
      <div class="value" id="water">--</div>
    </div>
  </div>
  <footer>© 2025 Smart Garden System</footer>

  <script>
    function refresh() {
      fetch('/api/data')
        .then(r => r.json())
        .then(data => {
          document.getElementById('dhtTemp').innerText = data.dhtTemp + '°C';
          document.getElementById('dhtHum').innerText = data.dhtHum + '%';
          document.getElementById('lm35').innerText = data.lm35Temp + '°C';
          document.getElementById('ldr').innerText = data.ldr;
          const waterEl = document.getElementById('water');
          waterEl.innerText = data.water;
          waterEl.className = data.water === 'Wet' ? 'value status-wet' : 'value status-dry';
        });
    }
    setInterval(refresh, 2000);
    window.onload = refresh;
  </script>
</body>
</html>
  )html";

  server.send(200, "text/html", html);
}

// === API: JSON Data ===
void handleApiData() {
  String json = "{";
  json += "\"dhtTemp\":" + addQuotes(sensorData.dhtTemp) + ",";
  json += "\"dhtHum\":" + addQuotes(sensorData.dhtHum) + ",";
  json += "\"lm35Temp\":" + addQuotes(sensorData.lm35Temp) + ",";
  json += "\"ldr\":\"" + sensorData.ldrValue + "\",";
  json += "\"water\":" + addQuotes(sensorData.waterStatus);
  json += "}";
  server.send(200, "application/json", json);
}

// === Setup ===
void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println();
  Serial.println("================================");
  Serial.println("     ESP32 Sensor Gateway       ");
  Serial.println("================================");
  Serial.println("📡 Connecting to WiFi...");

  WiFi.begin(WIFI_SSID, WIFI_PASS);

  int timeout = 0;
  while (WiFi.status() != WL_CONNECTED && timeout < 20) {
    delay(500);
    Serial.print("• ");
    timeout++;
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\n❌ WiFi Failed!");
    while (true) delay(1);
  }

  Serial.println("\n✅ WiFi Connected!");
  Serial.print("📶 IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.println("================================");

  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/data", HTTP_GET, handleApiData);
  server.begin();
  Serial.println("🚀 Web server running.");
}

// === Main Loop ===
void loop() {
  server.handleClient();

  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    if (input.startsWith("T1:")) {
      Serial.println("[Serial] Received: " + input);
      sensorData.dhtTemp = parseValue(input, "T1");
      sensorData.dhtHum = parseValue(input, "H");
      sensorData.lm35Temp = parseValue(input, "T2");
      sensorData.ldrValue = parseValue(input, "L");
      sensorData.waterStatus = parseValue(input, "W");
      updateFirebase();
    }
  }
}