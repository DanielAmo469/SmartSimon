#include <Arduino.h> 
#include <stdint.h>
#include <vector>

#define BTN_1 18
#define BTN_2 19
#define BTN_3 23
#define BTN_4 33
#define BTN_5 27

#define LED_1 5
#define LED_2 21
#define LED_3 22
#define LED_4 32
#define LED_5 26

int selectedFolder = 1;  // Default folder for sounds
std::vector<int> sequence;  // Stores Simon's sequence
int playerIndex = 0;  // Tracks player's progress
int score = 0;  // Player's score

#define Start_Byte 0x7E
#define Version_Byte 0xFF
#define Command_Length 0x06
#define End_Byte 0xEF
#define Acknowledge 0x00  // No feedback from DFPlayer

// Button and LED mappings
const int buttons[] = {BTN_1, BTN_2, BTN_3, BTN_4, BTN_5};
const int leds[] = {LED_1, LED_2, LED_3, LED_4, LED_5};

// Function declarations
void execute_CMD(uint8_t CMD, uint8_t Par1, uint8_t Par2);
void playInFolder(int fold, int track);
void startGame();
void simonTurn();
void playerTurn();
void gameOver();
bool checkButtonPress(int &pressedButton);


void play() {
    execute_CMD(0x0D, 0, 1);
    delay(500);
  }
  
  void setVolume(int volume) {
    execute_CMD(0x06, 0, volume);  // Set the volume (0x00~0x30)
  
    delay(2000);
  }
  
  void playInFolder(int fold, int track) {  // upped data = folder + 4 upper track bits
    int upper = fold * 16 + track / 256;
    int lower = track % 256;  // lower data = 8 lower track bits
    execute_CMD(0x14, upper, lower);
    delay(500);
  }
  
  
  void execute_CMD(byte CMD, byte Par1, byte Par2)
  // Excecute the command and parameters
  {
    // Calculate the checksum (2 bytes)
    word checksum = -(Version_Byte + Command_Length + CMD + Acknowledge + Par1 + Par2);
    // Build the command line
    byte Command_line[10] = { Start_Byte, Version_Byte, Command_Length, CMD, Acknowledge,
                              Par1, Par2, highByte(checksum), lowByte(checksum), End_Byte };
    //Send the command line to the module
    for (byte k = 0; k < 10; k++) {
      Serial2.write(Command_line[k]);
    }
  }
  

// Game setup
void setup() {
    Serial2.begin(9600, SERIAL_8N1, 16, 17);
    Serial.begin(115200);
    setVolume(20);

    // Initialize LEDs and buttons
    for (int i = 0; i < 5; i++) {
        pinMode(leds[i], OUTPUT);
        pinMode(buttons[i], INPUT_PULLUP);
    }

    delay(1500);  // Allow DFPlayer time to initialize
    Serial.println(F("Enter folder number (1-7) to select sounds from:"));
}

void loop() {
    if (Serial.available()) {
        int newFolder = Serial.parseInt();
        if (newFolder >= 1 && newFolder <= 7) {
            selectedFolder = newFolder;
            Serial.print(F("Selected folder: "));
            Serial.println(selectedFolder);
            Serial.println(F("Press any button to start the game!"));
        } else {
            Serial.println(F("Invalid folder! Enter a number between 1 and 7."));
        }
    }

    // Wait for a button press to start the game
    int pressedButton;
    if (checkButtonPress(pressedButton)) {
        startGame();
    }
}

// Function to start the game
void startGame() {
    sequence.clear();  // Reset sequence
    score = 0;  // Reset score
    playerIndex = 0;
    Serial.println(F("Game Started! Watch Simon..."));
    delay(1000);
    
    simonTurn();
}

// Function for Simon's turn (plays sequence)
void simonTurn() {
    // Add a new random move
    int newMove = random(0, 5);  // Pick a random button
    sequence.push_back(newMove);

    // Play the sequence
    for (int move : sequence) {
        digitalWrite(leds[move], HIGH);
        playInFolder(selectedFolder, move + 1);
        delay(800);
        digitalWrite(leds[move], LOW);
        delay(300);
    }

    Serial.println(F("Now it's your turn!"));
    playerIndex = 0;
    playerTurn();
}

// Function for player's turn
void playerTurn() {
    int pressedButton;
    while (playerIndex < sequence.size()) {
        if (checkButtonPress(pressedButton)) {
            if (pressedButton == sequence[playerIndex]) {
                // Correct move
                digitalWrite(leds[pressedButton], HIGH);
                playInFolder(selectedFolder, pressedButton + 1);
                delay(300);
                digitalWrite(leds[pressedButton], LOW);
                playerIndex++;
            } else {
                // Incorrect move
                gameOver();
                return;
            }
        }
    }

    // Player completed sequence, award points and continue
    score += 10;
    Serial.print(F("Correct! Score: "));
    Serial.println(score);
    delay(1000);
    simonTurn();  // Extend the sequence and play again
}

// Function to check button press
bool checkButtonPress(int &pressedButton) {
    for (int i = 0; i < 5; i++) {
        if (digitalRead(buttons[i]) == LOW) {
            delay(50);  // Simple debounce
            while (digitalRead(buttons[i]) == LOW);  // Wait for release
            pressedButton = i;
            return true;
        }
    }
    return false;
}

// Function for game over sequence
void gameOver() {
    Serial.println(F("Game Over!"));
    Serial.print(F("Final Score: "));
    Serial.println(score);

    // Blink all LEDs 3 times
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

    Serial.println(F("Press any button to restart."));
}