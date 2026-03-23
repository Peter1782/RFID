#include <SPI.h>
#include <MFRC522.h>

// Pines ESP32 -> RC522
#define SS_PIN 23
#define RST_PIN 18
#define SCK_PIN 22
#define MISO_PIN 19
#define MOSI_PIN 21

MFRC522 mfrc522(SS_PIN, RST_PIN);

void printArray(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

void setup() {
  Serial.begin(9600);
  delay(1000);

  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);
  mfrc522.PCD_Init();

  Serial.println("Sistema RFID listo");
  Serial.println("Acerca una tarjeta...");
}

void loop() {

  if (!mfrc522.PICC_IsNewCardPresent())
    return;

  if (!mfrc522.PICC_ReadCardSerial())
    return;

  Serial.print("Card UID:");
  printArray(mfrc522.uid.uidByte, mfrc522.uid.size);
  Serial.println();

  mfrc522.PICC_HaltA();
  delay(500);
}