#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <vector>
#include <WebServer.h>
#include <stdint.h>

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

// DFPlayer Mini Commands
#define Start_Byte 0x7E
#define Version_Byte 0xFF
#define Command_Length 0x06
#define End_Byte 0xEF
#define Acknowledge 0x00

// ✅ Wi-Fi and Backend Configuration
const char *targetSSID = "Daniel’s iPhone";                  // Update with your SSID
const char *password = "12345678";                           // Update with your password
const char *serverUrl = "http://172.20.10.11:8000/esp-data"; // FastAPI endpoint

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
String userID = ""; // Stores user ID after successful login
String username = "";
bool isLoggedIn = false;

// ✅ Define Button & LED Arrays
const int buttons[] = {BTN_1, BTN_2, BTN_3, BTN_4, BTN_5};
const int leds[] = {LED_1, LED_2, LED_3, LED_4, LED_5};

void execute_CMD(uint8_t CMD, uint8_t Par1, uint8_t Par2);
void playInFolder(int fold, int track);
void updateLCD(const char *line1, const char *line2);
void chooseSound();
void startGame();
void simonTurn();
void playerTurn();
void gameOver();
bool checkButtonPress(int &pressedButton);
void ledCycleLoop();
void waitForStart();
void askForLogin();
void handleLoginRequest();
void submitScore(int score);
void checkPing();

void setVolume(int volume)
{
    execute_CMD(0x06, 0, volume);
    delay(200);
}

// ✅ Function to update LCD screen
void updateLCD(const char *line1, const char *line2)
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(line1);
    lcd.setCursor(0, 1);
    lcd.print(line2);
}

void execute_CMD(uint8_t CMD, uint8_t Par1, uint8_t Par2)
{
    int16_t checksum = -(Version_Byte + Command_Length + CMD + Acknowledge + Par1 + Par2);
    uint8_t Command_line[10] = {Start_Byte, Version_Byte, Command_Length, CMD, Acknowledge,
                                Par1, Par2, highByte(checksum), lowByte(checksum), End_Byte};
    for (uint8_t k = 0; k < 10; k++)
    {
        Serial2.write(Command_line[k]);
    }
}

void playInFolder(int fold, int track)
{
    int upper = fold * 16 + track / 256;
    int lower = track % 256;
    execute_CMD(0x14, upper, lower);
    delay(500);
}

// ✅ Function to check button press (Debounce)
bool checkButtonPress(int &pressedButton)
{
    for (int i = 0; i < 5; i++)
    {
        if (digitalRead(buttons[i]) == LOW)
        {
            delay(150);
            while (digitalRead(buttons[i]) == LOW)
                ;
            pressedButton = i;
            return true;
        }
    }
    return false;
}

void chooseSound()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Choose Sound:");
    lcd.setCursor(0, 1);
    lcd.print("Press a button");

    // ✅ Turn on all LEDs to indicate selection mode
    for (int i = 0; i < 5; i++)
    {
        digitalWrite(leds[i], HIGH);
    }

    Serial.println("🎵 Choose a sound by pressing a button:");
    Serial.println("🔴 Red    -> Classic");
    Serial.println("⚪ White  -> Dogs");
    Serial.println("🟢 Green  -> Cats");
    Serial.println("🟣 Purple -> Harp");
    Serial.println("🟡 Yellow -> Violin");

    while (true)
    {
        int pressedButton;
        delay(200);
        if (checkButtonPress(pressedButton))
        {
            // ✅ Map buttons to specific folders
            switch (pressedButton)
            {
            case 3:
                selectedFolder = 1;
                break; // Red -> Classic
            case 2:
                selectedFolder = 3;
                break; // White -> Dogs
            case 1:
                selectedFolder = 4;
                break; // Green -> Cats
            case 0:
                selectedFolder = 5;
                break; // Purple -> Violin
            case 4:
                selectedFolder = 6;
                break; // Yellow -> Piano
            default:
                selectedFolder = 1;
                break; // Fallback
            }

            // ✅ Define folder names
            String folderName;
            switch (selectedFolder)
            {
            case 1:
                folderName = "Classic";
                break;
            case 3:
                folderName = "Dogs";
                break;
            case 4:
                folderName = "Cats";
                break;
            case 5:
                folderName = "Harp";
                break;
            case 6:
                folderName = "Violin";
                break;
            }

            // ✅ Turn off all LEDs
            for (int i = 0; i < 5; i++)
            {
                digitalWrite(leds[i], LOW);
            }

            // ✅ Light up only the selected LED for 5 seconds
            digitalWrite(leds[pressedButton], HIGH);

            // ✅ Display the selection on LCD (No Emojis)
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Sound Selected:");
            lcd.setCursor(0, 1);
            lcd.print(folderName);

            Serial.print("✅ Sound Folder ");
            Serial.print(selectedFolder);
            Serial.print(" (");
            Serial.print(folderName);
            Serial.println(") selected!");

            delay(5000); // Keep LED on for 5 seconds

            // ✅ Turn off the selected LED
            digitalWrite(leds[pressedButton], LOW);

            return; // Exit function after selection
        }
    }
}

// ✅ Function to start the game (MISSING DEFINITION FIXED)
void startGame()
{
    sequence.clear();
    score = 0;
    playerIndex = 0;
    delayBetweenSteps = 800;
    updateLCD("Game Started!", "Watch Simon");
    delay(1000);
    simonTurn();
}

void waitForStart()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Press a button");
    lcd.setCursor(0, 1);
    lcd.print("to start game!");

    Serial.println("🎮 Waiting for user to start the game...");

    while (true)
    {
        ledCycleLoop();
        int pressedButton;
        if (checkButtonPress(pressedButton))
        {
            if (selectedFolder == 0)
            {
                Serial.println("⚠️ No folder selected! Asking again...");
                chooseSound();
            }
            startGame();
            return;
        }
    }
}

void simonTurn()
{
    sequence.push_back(random(0, 5));
    updateLCD(("Score: " + String(score)).c_str(), "Simon's Turn");

    for (int move : sequence)
    {
        digitalWrite(leds[move], HIGH);
        playInFolder(selectedFolder, move + 1);
        delay(delayBetweenSteps);
        digitalWrite(leds[move], LOW);
        delay(300);
    }

    delayBetweenSteps = max(200, delayBetweenSteps - (score / 10));
    updateLCD(("Score: " + String(score)).c_str(), "Your Turn");
    playerIndex = 0;
    playerTurn();
}

void playerTurn()
{
    int pressedButton;
    while (playerIndex < sequence.size())
    {
        if (checkButtonPress(pressedButton))
        {
            if (pressedButton == sequence[playerIndex])
            {
                digitalWrite(leds[pressedButton], HIGH);
                playInFolder(selectedFolder, pressedButton + 1);
                delay(300);
                digitalWrite(leds[pressedButton], LOW);
                playerIndex++;
            }
            else
            {
                gameOver();
                return;
            }
        }
    }

    score += 10;
    Serial.print(F("Correct! Score: "));
    Serial.println(score);
    delay(1000);
    simonTurn();
}

void gameOver()
{
    Serial.println(F("❌ Game Over!"));
    Serial.print(F("🏆 Final Score: "));
    Serial.println(score);
    updateLCD("Game Over!", ("Score: " + String(score)).c_str());

    // ✅ Send score to FastAPI
    submitScore(score);

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 5; j++)
        {
            digitalWrite(leds[j], HIGH);
        }
        delay(500);
        for (int j = 0; j < 5; j++)
        {
            digitalWrite(leds[j], LOW);
        }
        delay(500);
    }

    // ✅ Ask if the user wants to change the sound
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Change Sound?");
    lcd.setCursor(0, 1);
    lcd.print("Yes:Ylw  No:Red");

    Serial.println("🎵 Do you want to change the sound?");
    Serial.println("🟡 Press Yellow for YES");
    Serial.println("🔴 Press Red for NO");

    digitalWrite(LED_5, HIGH); // Yellow LED ON (Yes)
    digitalWrite(LED_4, HIGH); // Red LED ON (No)

    while (true)
    {
        if (digitalRead(BTN_5) == LOW)
        { // Yellow button pressed (Change Sound)
            Serial.println("🎵 User wants to change the sound.");
            digitalWrite(LED_5, LOW);
            digitalWrite(LED_4, LOW);
            delay(1000);
            chooseSound();
            break;
        }

        if (digitalRead(BTN_4) == LOW)
        { // Red button pressed (Continue Playing)
            Serial.println("▶️ User chose to continue playing.");
            digitalWrite(LED_5, LOW);
            digitalWrite(LED_4, LOW);
            break;
        }
        delay(200);
    }

    // ✅ Restart the game process
    delay(2000);
    waitForStart();
}

void ledCycleLoop()
{
    for (int i = 0; i < 5; i++)
    {
        digitalWrite(leds[i], HIGH);
        delay(150);
        digitalWrite(leds[i], LOW);
    }
}

// ✅ Ask User if They Want to Log In
void askForLogin()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Login via Web?");
    lcd.setCursor(0, 1);
    lcd.print("Yes:Ylw  No:Red");

    digitalWrite(LED_5, HIGH); // Yellow LED ON (Yes)
    digitalWrite(LED_4, HIGH); // Red LED ON (No)

    while (true)
    {
        if (digitalRead(BTN_5) == LOW)
        { // Yellow button pressed (Log in)
            Serial.println("✅ Waiting for Web Login...");
            digitalWrite(LED_5, LOW);
            digitalWrite(LED_4, LOW);
            lcd.clear();
            lcd.print("Waiting for login...");

            // ✅ Wait until the user logs in
            while (!isLoggedIn)
            {
                server.handleClient();
                delay(100);
            }

            // ✅ Once logged in, print username and ask for sound selection
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Hello, ");
            lcd.print(username);
            lcd.setCursor(0, 1);
            lcd.print("ID: ");
            lcd.print(userID);

            Serial.println("✅ User Logged In: " + username + " (ID: " + userID + ")");
            delay(2000);

            // ✅ Ask user to choose a sound after login
            chooseSound();
            delay(1000);
            waitForStart(); // ensures LED loop before starting
            return;
        }

        if (digitalRead(BTN_4) == LOW)
        { // Red button pressed (Offline Mode)
            Serial.println("❌ User chose NOT to log in.");
            digitalWrite(LED_5, LOW);
            digitalWrite(LED_4, LOW);
            lcd.clear();
            lcd.print("Playing Offline");
            delay(2000);

            // ✅ Ask user to choose a sound before playing in offline mode
            chooseSound();
            delay(1000);
            waitForStart(); // ensures LED loop before starting
            return;
        }
        delay(200);
    }
}

// ✅ Handle Login Data from Web App
void handleLoginRequest()
{
    String body = server.arg("plain");
    Serial.println("📩 Received Login Data: " + body);

    JsonDocument doc;
    deserializeJson(doc, body);
    userID = doc["user_id"].as<String>();
    username = doc["username"].as<String>();
    isLoggedIn = true;

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Hello, ");
    lcd.print(username);
    lcd.setCursor(0, 1);
    lcd.print("ID: ");
    lcd.print(userID);

    Serial.println("✅ User Logged In: " + username + " (ID: " + userID + ")");
    server.send(200, "text/plain", "Login Data Received");

    // ✅ Ask user to choose a sound before starting the game
    delay(2000);
    chooseSound(); // This now ensures sound selection happens before game starts
    delay(1000);
    waitForStart();
}

// ✅ Check Backend Connection (Ping)
void checkPing()
{
    Serial.println("🔍 Pinging Backend...");
    IPAddress backendIP;
    if (WiFi.hostByName("172.20.10.1", backendIP))
    {
        Serial.print("✅ Backend reachable at: ");
        Serial.println(backendIP);
    }
    else
    {
        Serial.println("❌ Backend is unreachable!");
    }
}

void submitScore(int score)
{
    if (!isLoggedIn)
    {
        Serial.println("❌ No user logged in. Skipping score upload.");
        return;
    }

    HTTPClient http;
    http.begin("http://172.20.10.11:8000/submit-score");
    http.addHeader("Content-Type", "application/json");

    String requestBody = "{\"user_id\": " + userID + ", \"score\": " + String(score) + "}";
    int httpResponseCode = http.POST(requestBody);

    if (httpResponseCode == 200)
    {
        Serial.println("✅ Score uploaded successfully!");
    }
    else
    {
        Serial.println("❌ Failed to upload score.");
    }
    http.end();
}

// ✅ Setup Function (Game + FastAPI Integration)
void setup()
{
    Serial.begin(115200);
    Serial2.begin(9600, SERIAL_8N1, 16, 17);

    setVolume(25);
    server.on("/set-volume", HTTP_POST, []() {
        String body = server.arg("plain");
        Serial.println("🔊 Volume request received: " + body);
    
        StaticJsonDocument<200> doc;
        DeserializationError error = deserializeJson(doc, body);
        if (error) {
            Serial.println("❌ Failed to parse JSON");
            server.send(400, "application/json", "{\"error\": \"Invalid JSON\"}");
            return;
        }
    
        int volume = doc["volume"];
        if (volume < 0 || volume > 30) {
            server.send(400, "application/json", "{\"error\": \"Volume must be between 0 and 30\"}");
            return;
        }
    
        setVolume(volume); // your existing function
        Serial.printf("✅ Volume set to %d\n", volume);
        server.send(200, "application/json", "{\"message\": \"Volume set successfully\"}");
    });

    // ✅ Initialize LCD
    Wire.begin(LCD_SDA, LCD_SCL);
    lcd.begin(16, 2);
    lcd.backlight();
    lcd.clear();
    updateLCD("Booting Up...", "");

    // ✅ Set button & LED pins
    for (int i = 0; i < 5; i++)
    {
        pinMode(leds[i], OUTPUT);
        pinMode(buttons[i], INPUT_PULLUP);
    }

    // ✅ Connect to Wi-Fi
    WiFi.mode(WIFI_STA);
    WiFi.begin(targetSSID, password);

    int retries = 0;
    while (WiFi.status() != WL_CONNECTED && retries < 20)
    {
        delay(1000);
        Serial.print("⏳ Connecting to Wi-Fi...");
        retries++;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("\n✅ Connected to Wi-Fi!");
        Serial.print("🌐 ESP32 IP Address: ");
        Serial.println(WiFi.localIP());

        delay(2000); // ✅ Allow time for stability
        checkPing();

        // ✅ Handle root request
        server.on("/", HTTP_GET, []()
                  { server.send(200, "text/plain", "ESP32 Web Server Running!"); });

        // ✅ Handle login request
        server.on("/esp-login", HTTP_POST, handleLoginRequest);

        server.begin();
        Serial.println("✅ ESP Web Server Started! Listening for login data...");
    }
    else
    {
        Serial.println("\n❌ Failed to connect to Wi-Fi.");
    }

    // ✅ Ask user for login
    askForLogin();
}
void loop()
{
    server.handleClient(); // ✅ Process incoming web requests

    // ✅ If no sound is chosen, ask again
    if (selectedFolder == 0)
    {
        Serial.println(F("⚠️ No sound selected! Asking again..."));
        chooseSound();
    }

    // ✅ Wait for user to press a button to start
    Serial.println(F("🔄 Press any button to start the game."));
    waitForStart();
}