void setup() {
  Serial.begin(9600);
  pinMode(2, OUTPUT);  // LED ejemplo
}

void loop() {
  if (Serial.available()) {

    int comando = Serial.readStringUntil('\n').toInt();

    switch (comando) {

      case 1:
        digitalWrite(2, HIGH);
        Serial.println("LED ENCENDIDO");
        break;

      case 2:
        digitalWrite(2, LOW);
        Serial.println("LED APAGADO");
        break;

      case 3:
        Serial.println("Haciendo otra accion");
        break;

      default:
        Serial.println("Comando no valido");
        break;
    }
  }
}