// הגדרת עוצמה (0–30)
execute_CMD(0x06, 0, volumeLevel);  // למשל: volumeLevel = 25

// ניגון שיר מתיקייה מסוימת
execute_CMD(0x14, folder, track);   // folder=1, track=2