#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 23
#define RST_PIN 18
#define SCK_PIN 22
#define MISO_PIN 19
#define MOSI_PIN 21

#define BUZZER_PIN 12

#define BLOCK 1

MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

enum Mode {
  IDLE,
  READ_MODE,
  WRITE_MODE
};

Mode currentMode = IDLE;
char writeData[32];

void setup() {
  Serial.begin(9600);
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);
  pinMode(BUZZER_PIN, OUTPUT);
  mfrc522.PCD_Init();
  beep(1);

  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;
}

bool waitForCard() {
  return (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial());
}

void beep(int times){
  for(int i = 0; i < times; i++){
    digitalWrite(BUZZER_PIN, HIGH);
    delay(200);
    digitalWrite(BUZZER_PIN, LOW);
    delay(200);
  }
}

void writeBlock(char* inputData) {

  if (!waitForCard()) {
    return;
  }

  byte dataBlock[16] = {0};

  for (int i = 0; i < 16 && inputData[i] != '\0'; i++) {
    dataBlock[i] = inputData[i];
  }

  MFRC522::StatusCode status;

  status = mfrc522.PCD_Authenticate(
      MFRC522::PICC_CMD_MF_AUTH_KEY_A,
      BLOCK,
      &key,
      &(mfrc522.uid)
  );

  if (status != MFRC522::STATUS_OK) {
    Serial.println("ERROR_AUTHENTICATION");
    beep(2);
    currentMode = IDLE;
    return;
  }

  status = mfrc522.MIFARE_Write(BLOCK, dataBlock, 16);

  if (status == MFRC522::STATUS_OK) {
    Serial.println("OK_WRITE");
    beep(1);
  } else {
    Serial.println("ERROR_WRITE");
    beep(2);
  }

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  currentMode = IDLE;
}

void readBlock() {

  if (!waitForCard()) {
    return;
  }

  byte buffer[18];
  byte size = sizeof(buffer);

  MFRC522::StatusCode status;

  status = mfrc522.PCD_Authenticate(
      MFRC522::PICC_CMD_MF_AUTH_KEY_A,
      BLOCK,
      &key,
      &(mfrc522.uid)
  );

  if (status != MFRC522::STATUS_OK) {
    Serial.println("ERROR_AUTHENTICATION");
    beep(2);
    currentMode = IDLE;
    return;
  }

  status = mfrc522.MIFARE_Read(BLOCK, buffer, &size);

  if (status == MFRC522::STATUS_OK) {
    for (int i = 0; i < 16; i++) {
      if (buffer[i] == 0) break;
      Serial.print((char)buffer[i]);
    }
    Serial.println();
    beep(1);
  } else {
    Serial.println("ERROR_READ");
    beep(2);
  }

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  currentMode = IDLE;
}

void handleSerial() {
  if (!Serial.available()) return;

  String command = Serial.readStringUntil('\n');

  if (command.startsWith("WRITE")) {
    command.substring(6).toCharArray(writeData, 32);
    currentMode = WRITE_MODE;
  }
  else if (command == "READ") {
    currentMode = READ_MODE;
  }
  else if (command == "STOP") {
    currentMode = IDLE;
  }
  else if (command == "ERROR") {
    beep(2);
  }
  else if (command == "RESET") {
    ESP.restart();
  }
}

void loop() {
  handleSerial();

  if (currentMode == READ_MODE) {
    readBlock();
  }
  else if (currentMode == WRITE_MODE) {
    writeBlock(writeData);
  }

  delay(5);
}
