#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

// ✅ Wi-Fi and Server Configuration
const char* targetSSID = "Daniel’s iPhone";  // Replace with your hotspot SSID
const char* password = "12345678";           // Replace with your hotspot password
const char* serverUrl = "http://172.20.10.11:8000/esp-data";  // FastAPI server URL

// ✅ Function to Connect to Wi-Fi
void connectToWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(targetSSID, password);

    int retries = 0;
    Serial.print("🔄 Connecting to Wi-Fi");

    while (WiFi.status() != WL_CONNECTED && retries < 30) {  // 🔼 Increased retry attempts
        delay(1000);
        Serial.print(".");
        retries++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n✅ Connected to Wi-Fi!");
        Serial.print("📡 ESP32 IP Address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\n❌ Failed to connect to Wi-Fi.");
        Serial.println("🛑 Wi-Fi not available. Continuing without internet.");
    }
}
void setup() {
    Serial.begin(115200);
    connectToWiFi();
}

void loop() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("🔄 Wi-Fi disconnected. Reconnecting...");
        WiFi.reconnect();
        delay(5000);
        return;
    }

    // ✅ HTTP POST Request to FastAPI
    HTTPClient http;
    Serial.println("🌐 Sending data to FastAPI...");

    if (http.begin(serverUrl)) {  // ✅ Ensure HTTPClient starts correctly
        http.addHeader("Content-Type", "application/json");

        String jsonData = "{\"message\": \"Hello from ESP32!\"}";
        int httpResponseCode = http.POST(jsonData);

        if (httpResponseCode > 0) {
            String response = http.getString();
            Serial.println("✅ Response from server: " + response);
        } else {
            Serial.print("❌ Error in sending POST: ");
            Serial.println(http.errorToString(httpResponseCode).c_str());
        }

        http.end();  // ✅ Properly close connection
    } else {
        Serial.println("❌ HTTPClient failed to start.");
    }

    delay(5000);  // Wait before the next request
}