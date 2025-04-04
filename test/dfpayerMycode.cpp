#include <esp32-hal.h>

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

int selectedFolder = 1;  // Default folder

#define Start_Byte 0x7E
#define Version_Byte 0xFF
#define Command_Length 0x06
#define End_Byte 0xEF
#define Acknowledge 0x00  //Returns info with command 0x41 [0x01: info, 0x00: no info]
#define ACTIVATED LOW

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

void checkButton(int buttonPin, int ledPin, int track) {
  bool butt_state = digitalRead(buttonPin);

  if (butt_state == LOW) {  // ✅ Button is pressed

    playInFolder(selectedFolder, track);

    digitalWrite(ledPin, HIGH);
    delay(1);  // ✅ Small delay for stable response
  } else {     // ✅ Button released
    digitalWrite(ledPin, LOW);
  }
}



void setup() {
  Serial2.begin(9600, SERIAL_8N1, 16, 17);
  setVolume(20);

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
