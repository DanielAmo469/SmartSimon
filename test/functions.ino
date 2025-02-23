/// only dfplayer code nedded

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


void setup() {
  Serial2.begin(9600, SERIAL_8N1, 16, 17);
  Serial.begin(115200);
  delay(1000);
  setVolume(20);
}

void loop() {
  for (int i = 1; i <= 7; i++) {
    Serial.println(i);
    playInFolder(i, 1);
    delay(3000);
  }
}