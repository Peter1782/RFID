#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 23
#define RST_PIN 18
#define SCK_PIN 22
#define MISO_PIN 19
#define MOSI_PIN 21

MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

byte block = 1; // bloque donde se guardará el dato

byte dataBlock[16] = {
  '4','9','4','4','1','4','7','9',
  ' ',' ',' ',' ',' ',' ',' ',' '
};

void setup() {
  Serial.begin(9600);
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);
  mfrc522.PCD_Init();

  Serial.println("Acerca la tarjeta para escribir el numero...");

  // Clave por defecto de tarjetas nuevas
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;
}

void loop() {

  if (!mfrc522.PICC_IsNewCardPresent()) return;
  if (!mfrc522.PICC_ReadCardSerial()) return;

  Serial.println("Tarjeta detectada");

  MFRC522::StatusCode status;

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

  // Escribir el número en la tarjeta
  status = mfrc522.MIFARE_Write(block, dataBlock, 16);

  if (status != MFRC522::STATUS_OK) {
    Serial.print("Error escribiendo: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
  } else {
    Serial.println("Numero 49441479 escrito correctamente");
  }

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  delay(2000);
}