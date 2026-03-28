# 📡 Sistema de Fichaje con RFID (RC522 + ESP32 + Raspberry Pi)

## 🧩 Descripción del Proyecto

Este proyecto consiste en un sistema de fichaje utilizando tarjetas RFID.
El sistema permite identificar usuarios mediante tarjetas RFID usando un lector **RC522**, conectado a un **microcontrolador ESP32**, que a su vez se comunica con una **Raspberry Pi** para procesar y enviar los datos a una plataforma de fichaje.

El objetivo es crear un sistema de control de accesos o registro de entradas/salidas de empleados de manera sencilla y económica.

---

## 🪪 ¿Qué es RFID?

Un sistema RFID es una tecnología de radiofrecuencia que permite identificar objetos o personas sin necesidad de contacto. Funciona mediante un lector y una tarjeta (tag) que se comunican a través de ondas de radio. La tarjeta no necesita batería, ya que se alimenta del campo electromagnético generado por el lector, y contiene un chip con memoria interna donde se almacena información.

En tarjetas como las MIFARE 1K, la memoria es de 1KB y está organizada en sectores y bloques. En estos bloques se guardan datos que el sistema utiliza para identificar al usuario. Cuando acercas la tarjeta al lector (RC522), este detecta la tarjeta y lee el dato almacenado.

## ⚙️ Tecnologías Utilizadas

* Microcontrolador ESP32
* Raspberry Pi Zero
* Módulo RFID RC522
* Comunicación serie (UART/USB)
* (Opcional) Plataforma de fichaje

---

## 🔌 Esquema y Circuitería

En esta sección se describen las conexiones necesarias entre los componentes.

### 📍 Conexiones principales

#### RC522 → ESP32 (SPI)

| RC522    | ESP32        |
| -------- | ------------ |
| SDA (SS) | Pin 23       |
| SCK      | Pin 22       |
| MOSI     | Pin 21       |
| MISO     | Pin 19       |
| IRQ      | No conectado |
| GND      | GND          |
| RST      | Pin 18       |
| 3.3V     | 3.3V ⚠️      |

> ⚠️ **Importante:** NO conectar a 5V, el módulo puede dañarse.

---

#### 🔊 BUZZER → ESP32

| Buzzer | ESP32  |
| ------ | ------ |
| (+)    | Pin 12 |
| (-)    | GND    |

---

#### ESP32 → Raspberry Pi

* Conexión mediante USB (comunicación serie)

---

### 🖼️ Fotos de la circuitería

![esp32_arduino](https://github.com/user-attachments/assets/baac2c80-9633-4c69-ac0f-b3dd7c43965a)
![WhatsApp Image 2026-03-26 at 22 55 50](https://github.com/user-attachments/assets/cd1acdc8-8da1-4a32-b138-9c450999aa5c)

---


## 🧠 Funcionamiento del Sistema

1. El ESP32 recibe una orden (lectura o escritura) enviada por el programa en Python que se ejecuta en la Raspberry Pi.
2. El ESP32 activa el modo correspondiente y espera hasta leer o escribir los datos en la tarjeta RFID.
3. El ESP32 envía los datos obtenidos a la Raspberry Pi.
4. La Raspberry Pi:

   * Procesa los datos
   * Registra la entrada/salida
   * (Opcional) Los guarda en una plataforma de fichaje

---

## 🚀 Instalación y Uso

### 1. ESP32

* Cargar el código en el ESP32 usando **Arduino IDE** o **Arduino CLI**
* Instalar la librería `MFRC522`

#### 🔹 Usando Arduino CLI

🧩 Instalar Arduino CLI en Raspberry Pi:

```bash
curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh
```

👉 Añadir al PATH:

```bash
echo 'export PATH=$PATH:/home/administrador/arduino/bin' >> ~/.bashrc
source ~/.bashrc
```

👉 Comprobar:
```bash
arduino-cli version
```

⚙️🌐 Crear configuración + añadir ESP32

```bash
arduino-cli config init
arduino-cli config add board_manager.additional_urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli core update-index
```

💥 IMPORTANTE (ANTES DE INSTALAR ESP32)

👉 En Raspberry Pi Zero es MUY recomendable crear swap antes:
```bash
sudo fallocate -l 1G /swapfile
sudo chmod 600 /swapfile
sudo mkswap /swapfile
sudo swapon /swapfile
```

👉 Hacerlo permanente:
```bash
echo '/swapfile none swap sw 0 0' | sudo tee -a /etc/fstab
```

👉 Comprobar:
```bash
free -h
```

📦 Instalar soporte ESP32:
```bash
arduino-cli core install esp32:esp32
```

👉 Verificar la instalación del soporte ESP32:
```bash
arduino-cli core list
```

📚 Instalar librerías necesarias:
```bash
arduino-cli lib install "MFRC522"
```

🔍  Detectar el puerto de la conexion arudino con la maquina:
```bash
arduino-cli board list
```

⚙️ Compilar:
```bash
arduino-cli compile --fqbn esp32:esp32:esp32 rfid
```

🚀 Subir al ESP32
```bash
arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32 rfid
```

#### 🔹 Usando Arduino IDE

🌐 Añadir configuración ESP32:

👉 Preferencias → “Gestor de URLs adicionales de tarjetas”:
```bash
 https://espressif.github.io/arduino-esp32/package_esp32_index.json
```

⚙️ Añadir Placa ESP32:

👉 Herramientas → Placa → Gestor de tarjetas:

Buscar:
```bash
 esp32
```
Y darle a Instalar

👉 Herramientas → Placa → ESP32 Arduino → 
Seleccionar placa:
```bash
ESP32 Dev Module
```

📚 Instalar librerías necesarias:

👉 Programa → Incluir librería → Gestionar biblioteca:
```bash
MFRC522
```

### 2. Raspberry Pi o PC

* Conectar el ESP32 por USB
* Ejecutar el script en Python que controla el ESP32 a través del puerto serie
  
#### 🔹 Pasos:

⚙️ Crear entorno virutal:
```bash
python -m venv nombre_entorno
```

▶️ Activar el entorno:
```bash
source mi_entorno/bin/activate
```

📦 Instalar dependencias desde requirements.txt:
```bash
pip install -r requirements.txt
```

🚀 Ejecutar programa:
```bash
python app.py
```

### 3. Uso

* Acercar una tarjeta RFID al lector
* Ver el registro en el programa Python

---

## 📊 Resultado Final

Video del sistema en funcionamiento:

[![Ver video](https://img.youtube.com/vi/UQ3dbhMY7F4/0.jpg)](https://youtu.be/UQ3dbhMY7F4)
---

## 👨‍💻 Autor

Pedro Javier Durán García

---
