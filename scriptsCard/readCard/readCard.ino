#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 23
#define RST_PIN 18
#define SCK_PIN 22
#define MISO_PIN 19
#define MOSI_PIN 21

MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

byte block = 1;  // mismo bloque donde escribimos

void setup() {
  Serial.begin(9600);
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);
  mfrc522.PCD_Init();

  Serial.println("Acerca la tarjeta para leer el numero...");

  // Clave por defecto
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;
}

void loop() {

  if (!mfrc522.PICC_IsNewCardPresent()) return;
  if (!mfrc522.PICC_ReadCardSerial()) return;

  Serial.println("Tarjeta detectada");

  MFRC522::StatusCode status;
  byte buffer[18];
  byte size = sizeof(buffer);

  // Autenticar bloque
  status = mfrc522.PCD_Authenticate(
      MFRC522::PICC_CMD_MF_AUTH_KEY_A,
      block,
      &key,
      &(mfrc522.uid)
  );

  if (status != MFRC522::STATUS_OK) {
    Serial.print("Error autenticando: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  // Leer bloque
  status = mfrc522.MIFARE_Read(block, buffer, &size);

  if (status != MFRC522::STATUS_OK) {
    Serial.print("Error leyendo: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
  } else {
    Serial.print("Numero guardado: ");

    for (int i = 0; i < 16; i++) {
      Serial.print((char)buffer[i]);
    }
    Serial.println();
  }

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  delay(2000);
}