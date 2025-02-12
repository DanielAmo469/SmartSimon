#include "DFRobotDFPlayerMini.h"

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

DFRobotDFPlayerMini myDFPlayer;
int selectedFolder = 1;  // Default folder

void checkButton(int buttonPin, int ledPin, int track);  

void setup() {
    Serial2.begin(9600, SERIAL_8N1, 16, 17);
    Serial.begin(115200);

    pinMode(LED_1, OUTPUT);
    pinMode(LED_2, OUTPUT);
    pinMode(LED_3, OUTPUT);
    pinMode(LED_4, OUTPUT);
    pinMode(LED_5, OUTPUT);

    pinMode(BTN_1, INPUT_PULLUP);
    pinMode(BTN_2, INPUT_PULLUP);
    pinMode(BTN_3, INPUT_PULLUP);
    pinMode(BTN_4, INPUT_PULLUP);
    pinMode(BTN_5, INPUT_PULLUP);

    delay(1500);  // ✅ Allow DFPlayer time to initialize
    Serial.println(F("Initializing DFPlayer..."));

    if (!myDFPlayer.begin(Serial2)) {  
        Serial.println(F("DFPlayer initialization failed!"));
        while (true);
    }

    Serial.println(F("DFPlayer Mini online."));
    myDFPlayer.volume(25);  // Set volume level (0-30)

    Serial.println(F("Enter folder number (1-7) to select sounds from:"));
}

void loop() {
    if (Serial.available()) {
        int newFolder = Serial.parseInt();
        if (newFolder >= 1 && newFolder <= 7) {
            selectedFolder = newFolder;
            Serial.print(F("Selected folder: "));
            Serial.println(selectedFolder);
        } else {
            Serial.println(F("Invalid folder! Enter a number between 1 and 7."));
        }
    }

    checkButton(BTN_1, LED_1, 1);
    checkButton(BTN_2, LED_2, 2);
    checkButton(BTN_3, LED_3, 3);
    checkButton(BTN_4, LED_4, 4);
    checkButton(BTN_5, LED_5, 5);
}

void checkButton(int buttonPin, int ledPin, int track) {
    bool butt_state = digitalRead(buttonPin);

    if (butt_state == LOW) {  // ✅ Button is pressed
        myDFPlayer.playFolder(selectedFolder, track);
        digitalWrite(ledPin, HIGH);
        delay(1);  // ✅ Small delay for stable response
    } else {  // ✅ Button released
        digitalWrite(ledPin, LOW);
    }
}