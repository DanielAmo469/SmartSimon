#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <vector>
#include <WebServer.h>

// ✅ Custom I2C Pins for LCD
#define LCD_SDA 13
#define LCD_SCL 14

// ✅ Buttons
#define BTN_1 18 // Purple
#define BTN_2 19 // Green
#define BTN_3 23 // White
#define BTN_4 33 // Red (No)
#define BTN_5 27 // Yellow (Yes)

// ✅ LEDs
#define LED_1 5  // Purple
#define LED_2 21 // Green
#define LED_3 22 // White
#define LED_4 32 // Red (No)
#define LED_5 26 // Yellow (Yes)

// ✅ Wi-Fi and Backend Configuration
const char* targetSSID = "Daniel’s iPhone";  // Update with your SSID
const char* password = "12345678";           // Update with your password
const char* serverUrl = "http://172.20.10.11:8000/esp-data";  // FastAPI endpoint

// ✅ Web Server (ESP32 listens for login data)
WebServer server(8000);

// ✅ LCD Setup
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ✅ Game Variables
int selectedFolder = 0;
std::vector<int> sequence;
int playerIndex = 0;
int score = 0;
int delayBetweenSteps = 800;
String userID = "";  // Stores user ID after successful login
String username = "";
bool isLoggedIn = false;

// ✅ Define Button & LED Arrays
const int buttons[] = {BTN_1, BTN_2, BTN_3, BTN_4, BTN_5};
const int leds[] = {LED_1, LED_2, LED_3, LED_4, LED_5};

// ✅ Function Declarations
void askForLogin();
void handleLoginRequest();
void updateLCD(const char *line1, const char *line2);
void submitScore(int score);
void startGame();
void simonTurn();
void playerTurn();
void gameOver();
bool checkButtonPress(int &pressedButton);
void ledCycleLoop();
void waitForStart();
void execute_CMD(uint8_t CMD, uint8_t Par1, uint8_t Par2);
void playInFolder(int fold, int track);
void checkPing();

// ✅ Function to update LCD screen
void updateLCD(const char *line1, const char *line2) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(line1);
    lcd.setCursor(0, 1);
    lcd.print(line2);
}

// ✅ Setup Function
void setup() {
    Serial.begin(115200);
    Serial2.begin(9600, SERIAL_8N1, 16, 17);

    // ✅ Initialize LCD
    Wire.begin(LCD_SDA, LCD_SCL);
    lcd.begin(16, 2);
    lcd.backlight();
    lcd.clear();

    // ✅ Set button & LED pins
    for (int i = 0; i < 5; i++) {
        pinMode(leds[i], OUTPUT);
        pinMode(buttons[i], INPUT_PULLUP);
    }

    // ✅ Connect to Wi-Fi
    WiFi.mode(WIFI_STA);
    WiFi.begin(targetSSID, password);

    int retries = 0;
    while (WiFi.status() != WL_CONNECTED && retries < 20) {
        delay(1000);
        Serial.print("⏳ Connecting to Wi-Fi...");
        retries++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n✅ Connected to Wi-Fi!");
        Serial.print("🌐 ESP32 IP Address: ");
        Serial.println(WiFi.localIP());

        delay(2000);  // ✅ Allow time for stability

        checkPing();

        // ✅ Handle root request
        server.on("/", HTTP_GET, []() {
            server.send(200, "text/plain", "ESP32 Web Server Running!");
        });

        // ✅ Handle login request
        server.on("/esp-login", HTTP_POST, handleLoginRequest);
        
        server.begin();  
        Serial.println("✅ ESP Web Server Started! Listening for login data...");
    } else {
        Serial.println("\n❌ Failed to connect to Wi-Fi.");
    }

    // ✅ Ask user for login
    askForLogin();
}

// ✅ Loop Function
void loop() {
    server.handleClient();
}

// ✅ Ask User if They Want to Log In
void askForLogin() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Login via Web?");
    lcd.setCursor(0, 1);
    lcd.print("Yes:Ylw  No:Red");

    digitalWrite(LED_5, HIGH);  // Yellow LED ON (Yes)
    digitalWrite(LED_4, HIGH);  // Red LED ON (No)

    while (true) {
        if (digitalRead(BTN_5) == LOW) {  // Yellow button pressed
            Serial.println("✅ Waiting for Web Login...");
            digitalWrite(LED_5, LOW);
            digitalWrite(LED_4, LOW);
            lcd.clear();
            lcd.print("Wait for login...");
            return;
        }

        if (digitalRead(BTN_4) == LOW) {  // Red button pressed (Offline Mode)
            Serial.println("❌ User chose NOT to log in.");
            digitalWrite(LED_5, LOW);
            digitalWrite(LED_4, LOW);
            lcd.clear();
            lcd.print("Playing Offline");

            // ✅ Start game immediately in offline mode
            delay(2000);
            startGame();
            return;
        }
        delay(200);
    }
}

// ✅ Handle Login Data from Web App
void handleLoginRequest() {
    String body = server.arg("plain");
    Serial.println("📩 Received Login Data: " + body);

    JsonDocument doc;
    deserializeJson(doc, body);
    userID = doc["user_id"].as<String>();
    username = doc["username"].as<String>();
    isLoggedIn = true;

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("User:");
    lcd.setCursor(6, 0);
    lcd.print(username);
    lcd.setCursor(0, 1);
    lcd.print("ID:");
    lcd.setCursor(3, 1);
    lcd.print(userID);

    Serial.println("✅ User Logged In: " + username + " (ID: " + userID + ")");
    server.send(200, "text/plain", "Login Data Received");

    // ✅ Start the game immediately after login
    delay(2000);
    startGame();
}

// ✅ Start Simon Game
void startGame() {
    sequence.clear();
    score = 0;
    playerIndex = 0;
    delayBetweenSteps = 800;

    updateLCD("Game Started!", "Watch Simon");
    delay(1000);
    simonTurn();
}

// ✅ ESP32's Turn
void simonTurn() {
    sequence.push_back(random(0, 5));
    updateLCD(("Score: " + String(score)).c_str(), "Simon's Turn");

    for (int move : sequence) {
        digitalWrite(leds[move], HIGH);
        playInFolder(selectedFolder, move + 1);  // ✅ This function must be defined!
        delay(delayBetweenSteps);
        digitalWrite(leds[move], LOW);
        delay(300);
    }

    delayBetweenSteps = max(300, delayBetweenSteps - 50);
    updateLCD("Your Turn", "");
    playerIndex = 0;
    playerTurn();
}

// ✅ Player's Turn
void playerTurn() {
    int pressedButton;
    while (playerIndex < sequence.size()) {
        if (checkButtonPress(pressedButton)) {
            if (pressedButton == sequence[playerIndex]) {
                digitalWrite(leds[pressedButton], HIGH);
                playInFolder(selectedFolder, pressedButton + 1);
                delay(300);
                digitalWrite(leds[pressedButton], LOW);
                playerIndex++;
            } else {
                gameOver();
                return;
            }
        }
    }

    score += 10;
    Serial.println("✅ Correct! Score: " + String(score));
    delay(1000);
    simonTurn();
}

// ✅ Play sound from folder (DFPlayer Mini)
void playInFolder(int fold, int track) {
    Serial.print("🎵 Playing sound from folder: ");
    Serial.print(fold);
    Serial.print(", Track: ");
    Serial.println(track);

    execute_CMD(0x14, fold, track);
    delay(500);
}

// ✅ Check for button press (Debounce)
bool checkButtonPress(int &pressedButton) {
    for (int i = 0; i < 5; i++) {
        if (digitalRead(buttons[i]) == LOW) {
            delay(150);
            while (digitalRead(buttons[i]) == LOW);
            pressedButton = i;
            return true;
        }
    }
    return false;
}

// ✅ Game Over Function
void gameOver() {
    Serial.println("❌ Game Over!");
    Serial.print("🏆 Final Score: ");
    Serial.println(score);
    updateLCD("Game Over!", ("Score: " + String(score)).c_str());

    // Flash all LEDs to indicate failure
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 5; j++) {
            digitalWrite(leds[j], HIGH);
        }
        delay(500);
        for (int j = 0; j < 5; j++) {
            digitalWrite(leds[j], LOW);
        }
        delay(500);
    }

    // Restart game after a short delay
    delay(2000);
    startGame();
}

// ✅ Check Backend Connection (Ping)
void checkPing() {
    Serial.println("🔍 Pinging Backend...");
    IPAddress backendIP;
    if (WiFi.hostByName("172.20.10.1", backendIP)) {
        Serial.print("✅ Backend reachable at: ");
        Serial.println(backendIP);
    } else {
        Serial.println("❌ Backend is unreachable!");
    }
}
