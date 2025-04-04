#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <stdint.h>
#include <vector>
#include <WiFi.h>
#include <HTTPClient.h>

// ✅ Custom I2C Pins for LCD
#define LCD_SDA 13
#define LCD_SCL 14

// Buttons
#define BTN_1 18 //Purple
#define BTN_2 19 //Green
#define BTN_3 23 //White
#define BTN_4 33 //Red
#define BTN_5 27 //Yellow

// LEDs
#define LED_1 5 //Purple
#define LED_2 21 //Green
#define LED_3 22 //White
#define LED_4 32 //Red
#define LED_5 26 //Yellow

// Wi-Fi and Server Configuration
const char* targetSSID = "Daniel’s iPhone";  // Replace with your SSID
const char* password = "12345678";           // Replace with your password
const char* serverUrl = "http://172.20.10.11:8000/esp-data";  // FastAPI endpoint


int selectedFolder = 0;
std::vector<int> sequence;
int playerIndex = 0;
int score = 0;
int delayBetweenSteps = 800;

// ✅ LCD Setup
LiquidCrystal_I2C lcd(0x27, 16, 2);

// DFPlayer Mini Commands
#define Start_Byte 0x7E
#define Version_Byte 0xFF
#define Command_Length 0x06
#define End_Byte 0xEF
#define Acknowledge 0x00

// Button and LED mappings
const int buttons[] = {BTN_1, BTN_2, BTN_3, BTN_4, BTN_5};
const int leds[] = {LED_1, LED_2, LED_3, LED_4, LED_5};

// Function declarations
void execute_CMD(uint8_t CMD, uint8_t Par1, uint8_t Par2);
void playInFolder(int fold, int track);
void updateLCD(const char *line1, const char *line2);
void startGame();
void simonTurn();
void playerTurn();
void gameOver();
bool checkButtonPress(int &pressedButton);
void ledCycleLoop();
void waitForStart();

void setVolume(int volume) {
    execute_CMD(0x06, 0, volume);
    delay(200);
}

void execute_CMD(uint8_t CMD, uint8_t Par1, uint8_t Par2) {
    int16_t checksum = -(Version_Byte + Command_Length + CMD + Acknowledge + Par1 + Par2);
    uint8_t Command_line[10] = {Start_Byte, Version_Byte, Command_Length, CMD, Acknowledge, 
                                Par1, Par2, highByte(checksum), lowByte(checksum), End_Byte};
    for (uint8_t k = 0; k < 10; k++) {
        Serial2.write(Command_line[k]);
    }
}

void playInFolder(int fold, int track) {
    int upper = fold * 16 + track / 256;
    int lower = track % 256;
    execute_CMD(0x14, upper, lower);
    delay(500);
}

// ✅ Function to update LCD screen (MISSING DEFINITION FIXED)
void updateLCD(const char *line1, const char *line2) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(line1);
    lcd.setCursor(0, 1);
    lcd.print(line2);
}

// ✅ Function to check button press (Debounce)
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

// ✅ Function to start the game (MISSING DEFINITION FIXED)
void startGame() {
    sequence.clear();
    score = 0;
    playerIndex = 0;
    delayBetweenSteps = 800;
    updateLCD("Game Started!", "Watch Simon");
    delay(1000);
    simonTurn();
}

// ✅ Game setup (setup() comes after all function definitions)
void setup() {
    Serial2.begin(9600, SERIAL_8N1, 16, 17);
    Serial.begin(115200);
    setVolume(20);
    delay(1500);

    // ✅ Initialize LCD with new I2C pins
    Wire.begin(LCD_SDA, LCD_SCL);
    lcd.begin(16, 2);
    lcd.backlight();
    lcd.clear();
    updateLCD("Select Folder", "1-7 via Serial");

    for (int i = 0; i < 5; i++) {
        pinMode(leds[i], OUTPUT);
        pinMode(buttons[i], INPUT_PULLUP);
    }
    WiFi.mode(WIFI_STA);
    WiFi.begin(targetSSID, password);
  
    // Wait for Wi-Fi connection
    int retries = 0;
    while (WiFi.status() != WL_CONNECTED && retries < 20) {
      delay(1000);
      Serial.print(".");
      retries++;
    }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nConnected to Wi-Fi!");
      Serial.print("ESP32 IP Address: ");
      Serial.println(WiFi.localIP());
    } else {
      Serial.println("\nFailed to connect to Wi-Fi.");
    }
  

    delay(1500);
    Serial.println(F("Enter folder number (1-7) to select sounds from:"));
}

void loop() {
    if (selectedFolder == 0) {
        if (Serial.available()) {
            int newFolder = Serial.parseInt();
            if (newFolder >= 1 && newFolder <= 7) {
                selectedFolder = newFolder;
                updateLCD("Folder Selected:", String(selectedFolder).c_str());
                Serial.println(F("Press any button to start the game!"));
                waitForStart();
            } else {
                Serial.println(F("Invalid folder! Enter 1-7."));
            }
        }
    } else {
        Serial.println(F("Enter new folder (1-7) or press a button to restart."));
        waitForStart();
    }
}

void waitForStart() {
    while (true) {
        ledCycleLoop();
        int pressedButton;
        if (checkButtonPress(pressedButton)) {
            startGame();
            return;
        }
        if (Serial.available()) {
            int newFolder = Serial.parseInt();
            if (newFolder >= 1 && newFolder <= 7) {
                selectedFolder = newFolder;
                updateLCD("Folder Changed:", String(selectedFolder).c_str());
                Serial.println(F("Press any button to start."));
            }
        }
    }
}

void simonTurn() {
    sequence.push_back(random(0, 5));
    updateLCD(("Score: " + String(score)).c_str(), "Simon's Turn");

    for (int move : sequence) {
        digitalWrite(leds[move], HIGH);
        playInFolder(selectedFolder, move + 1);
        delay(delayBetweenSteps);
        digitalWrite(leds[move], LOW);
        delay(300);
    }

    delayBetweenSteps = max(300, delayBetweenSteps - 50);
    updateLCD(("Score: " + String(score)).c_str(), "Your Turn");
    playerIndex = 0;
    playerTurn();
}

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
    Serial.print(F("Correct! Score: "));
    Serial.println(score);
    delay(1000);
    simonTurn();
}

void gameOver() {
    Serial.println(F("Game Over!"));
    Serial.print(F("Final Score: "));
    Serial.println(score);
    updateLCD("Game Over!", ("Score: " + String(score)).c_str());

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
}

void ledCycleLoop() {
    for (int i = 0; i < 5; i++) {
        digitalWrite(leds[i], HIGH);
        delay(150);
        digitalWrite(leds[i], LOW);
    }
}