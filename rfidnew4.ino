#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

// pin definitions
#define RST_PIN 9
#define SS_PIN 10

MFRC522 rfid(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2);

// tag db
struct Tag {
  const char *uid;
  const char *name;
  const char *role;
};

const Tag allowedTags[] = {
  {"8A6F951A", "Admin Card", "Admin"},
  {"5DC5B001", "Raine Aspe", "Student"},
  {"0BD5B101", "John Walter", "Student"}, //red
  {"627BB701", "Thomas Manzanilla", "Student"}, //purple
  {"2E7A0D02", "Dawn Marbida", "Student"},
  {"5A6B4116", "Shania Ante", "Student"}, //yellow
  {"17DF0D02", "Marean Ante", "Student"} //orange
};
const byte NUM_TAGS = sizeof(allowedTags) / sizeof(allowedTags[0]);

// global vars
char tagID[16]; // holds scanned UID


// init function
void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();
  delay(100);

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print("Access Control");
  lcd.setCursor(0, 1);
  lcd.print("Scan your card");

  Serial.println(F("System ready. Waiting for card..."));
}

// reads rfid uid
bool readCard() {
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial())
    return false;

  byte len = 0;
  for (byte i = 0; i < rfid.uid.size && len < sizeof(tagID) - 1; i++) {
    sprintf(&tagID[len], "%02X", rfid.uid.uidByte[i]);
    len += 2;
  }
  tagID[len] = '\0';
  rfid.PICC_HaltA();

  Serial.print(F("Detected UID: "));
  Serial.println(tagID);
  return true;
}

// finds tag info
const Tag* findTag(const char *uid) {
  for (byte i = 0; i < NUM_TAGS; i++) {
    if (strcasecmp(uid, allowedTags[i].uid) == 0)
      return &allowedTags[i];
  }
  return nullptr;
}
 // prints message to lcd
void showLCD(const char *line1, const char *line2, unsigned long duration = 3000) {
  lcd.clear();
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
  delay(duration);
  lcd.clear();
  lcd.print("Scan your card");
}

// main loop
void loop() {
  if (!readCard()) return;

  const Tag *tag = findTag(tagID);

  if (tag) {
    Serial.print(F("Access Granted: "));
    Serial.print(tag->name);
    Serial.print(F(" ("));
    Serial.print(tag->role);
    Serial.println(F(")"));

    if (strcmp(tag->role, "Student") == 0)
      showLCD("Student Detected", tag->name);
    else
      showLCD("Access Granted", tag->name);
  } else {
    Serial.println(F("Access Denied!"));
    showLCD("Access Denied!", "");
  }

  Serial.println(F("--------------------------"));
}
