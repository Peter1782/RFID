import customtkinter as ctk
from tkinter import ttk
import threading
import time
import serial
import requests
import json
from datetime import datetime
import logging
from dotenv import load_dotenv
import os
from requests.exceptions import Timeout, RequestException

load_dotenv()

# ----------------------------
# CONFIGURACION LOGS
# ----------------------------
logging.basicConfig(level=logging.INFO, format="%(asctime)s | %(levelname)s | %(threadName)s | %(message)s",
                    handlers=[logging.FileHandler(
                        "app.log", encoding="utf-8"), logging.StreamHandler()]
                    )

logging.info("Start...")

# ----------------------------
# CONEXION ARDUINO
# ----------------------------
#arduino = serial.Serial('COM4', 9600, timeout=1)
arduino = serial.Serial('/dev/ttyUSB0', 9600, timeout=1)
time.sleep(2)
running = False
arduino_lock = threading.Lock()

# ----------------------------
# PETICIONES API
# ----------------------------
session = requests.Session()

user_login = os.getenv("USER_LOGIN")
password = os.getenv("PASSWORD")


def list_user():
    try:
        url = os.getenv("URL_WORKERS")

        payload = json.dumps({
            "login": user_login,
            "password": password
        })

        headers = {'Content-Type': 'application/json'}

        logging.info("Send %s", url)
        response = requests.post(
             url, headers=headers, data=payload,  verify=False, timeout=(2, 2))
        logging.info("status: %s", response.status_code)
        
        if response.status_code == 200:
         return response.json()

    except Timeout:
        logging.error("Error Timeout connection")

    except RequestException as e:
        logging.error("Error connection: %s", e)


def login():
    login_url = os.getenv("URL_LOGIN")

    payload = {
        "userid": user_login,
        "password": password
    }

    headers = {"Content-Type": "application/x-www-form-urlencoded"}

    logging.info("Send %s", login_url)
    response = session.post(login_url, data=payload,
                            headers=headers, timeout=(2, 2))

    logging.info("status: %s", response.status_code)
    logging.info("Cookie: %s", session.cookies.get_dict())


def manual_verification(code):
    try:
        login()
        url = os.getenv("URL_VERIFICATION")

        now = datetime.now()

        date = now.strftime("%d/%m/%Y")
        time = now.strftime("%H:%M:%S")

        payload = {
            "date": date,
            "time": time,
            "mode": "codeList",
            "code": code,
            "createCommand": "Insertar"
        }

        logging.info("Send %s", url)

        headers = {"Content-Type": "application/x-www-form-urlencoded"}

        response = session.post(url, data=payload, headers=headers, auth=(
            user_login, password), timeout=(2, 2))

        logging.info("status: %s", response.status_code)

        if response.status_code == 200 and "Insertada verificación" in response.text:
            return code

        return "ERROR_REGISTER_CHECK"

    except Timeout:
        logging.error("Error Timeout connection")
        return "ERROR_TIMEOUT"

    except RequestException as e:
        logging.error("Error connection: %s", e)
        return "ERROR"



# ----------------------------
# COMUNICACION NFC
# ----------------------------
def operation_nfc(type, value):
    global running
    global arduino_lock
    try:
        while running:
            with arduino_lock:
                arduino.reset_input_buffer()

                if type == "R":
                    arduino.write(b"READ\n")

                elif type == "W":
                    arduino.write(f"WRITE:{value}\n".encode())

            start = time.time()

            while time.time() - start < 0.5:
                if not running:
                    return "TERMINATE"

                with arduino_lock:
                    if arduino.in_waiting:
                        linea = arduino.readline().decode(errors="ignore").strip()
                        return linea

                time.sleep(0.02)

            time.sleep(0.1)
    except Exception as e:
        logging.error("Error NFC: %s", e)
        return "ERROR_NFC"


def error_nfc():
    with arduino_lock:
        arduino.write(b"ERROR\n")


def started_nfc():
    with arduino_lock:
        arduino.write(b"STARTED\n")


# ----------------------------
# CONFIGURACION APP
# ----------------------------
app = ctk.CTk()
app.title("Sistema NFC")
app.geometry("500x400")

ctk.set_appearance_mode("dark")
ctk.set_default_color_theme("blue")

title = ctk.CTkLabel(app, text="Sistema de Gestión NFC",
                     font=("Arial", 20, "bold"))
title.pack(pady=30)


# ----------------------------
# ESCRIBIR NFC
# ----------------------------
def window_write():
    global running
    if not running:
        logging.info("Window write open")

        running = True

        window = ctk.CTkToplevel(app)
        window.title("Escribir NFC")
        window.geometry("600x700")
        window.attributes("-topmost", True)

        label = ctk.CTkLabel(
            window, text="Selecciona un usuario", font=("Arial", 18))
        label.pack(pady=10)

        def close():
            global running
            running = False
            window.destroy()

        window.protocol("WM_DELETE_WINDOW", close)

        style = ttk.Style()
        style.theme_use("default")
        style.configure("Treeview",
                        background="#2b2b2b",
                        foreground="white",
                        rowheight=28,
                        fieldbackground="#2b2b2b")
        style.map("Treeview", background=[("selected", "#1f6aa5")])

        table = ttk.Treeview(window, columns=(
            "Nombre", "DNI"), show="headings", selectmode="browse")

        table.heading("Nombre", text="Nombre")
        table.heading("DNI", text="DNI")

        table.column("Nombre", width=250)
        table.column("DNI", width=150)

        users = list_user()

        if not users:
            error_nfc()
        else:
            for user in users:
                name = user.get("nombre", "")
                lastName = user.get("apellidos", "")
                code = user.get("codigo", "")

                fullName = f"{name} {lastName}".strip()

                table.insert("", "end", values=(fullName, code))

        table.pack(pady=20, fill="both", expand=True)

        label_result = ctk.CTkLabel(
            window, text="Seleccione un usuario y pulse el botón", font=("Arial", 16))
        label_result.pack(pady=5)

        def write_user():
            selected = table.selection()

            if not selected:
                label_result.configure(text="Seleccione primero un usuario.")
                return

            button_write.configure(state="disabled")

            data = table.item(selected)["values"]
            code = data[1]
            logging.info("Code selection: %s", code)

            label_result.configure(text="Acerque la tarjeta...")

            def task():
                logging.info("Loading Write Arduino...")
                data = operation_nfc("W", code)

                logging.info("Response arduino: %s", data)

                if not running:
                    return

                if data == "OK_WRITE":
                    window.after(
                        0, lambda: label_result.configure(text=f"{code} ✅"))
                else:
                    window.after(
                        0, lambda: label_result.configure(text=f"{data} ❌"))

                time.sleep(2)

                if running:
                    window.after(0, lambda: label_result.configure(
                        text="Seleccione un usuario y pulse el botón"))
                    window.after(
                        0, lambda: button_write.configure(state="normal"))

            threading.Thread(target=task, daemon=True).start()

        button_write = ctk.CTkButton(
            window, text="Escribir en NFC", command=write_user)
        button_write.pack(pady=15)



# ----------------------------
# LEER NFC
# ----------------------------
def window_read():
    global running
    if not running:
        logging.info("Window read open")
        running = True

        window = ctk.CTkToplevel(app)
        window.title("Leer NFC")
        window.geometry("500x400")
        window.attributes("-topmost", True)

        label = ctk.CTkLabel(window, text="Leyendo NFC...", font=("Arial", 18))
        label.pack(pady=20)

        progress = ctk.CTkProgressBar(window, mode="indeterminate")
        progress.pack(pady=20)
        progress.start()

        def close():
            global running
            running = False
            progress.stop()
            window.destroy()

        window.protocol("WM_DELETE_WINDOW", close)

        def task():
            global running

            while running:
                logging.info("Loading Read Arduino...")
                data = operation_nfc("R", None)

                logging.info("Response arduino: %s", data)

                if not running:
                    break

                window.after(0, lambda: label.configure(text="Procesando..."))

                time.sleep(2)

                if "ERROR" in data:
                    window.after(
                        0, lambda d=data: label.configure(text=f"{d} ❌"))
                else:
                    response = manual_verification(data)

                    logging.info("Response verification: %s", response)

                    if "ERROR" in response:
                        window.after(
                            0, lambda r=response: label.configure(text=f"{r} ❌"))
                        error_nfc()
                    else:
                        window.after(
                            0, lambda d=data: label.configure(text=f"{d} ✅"))

                time.sleep(1)

                if running:
                    window.after(0, lambda: label.configure(
                        text="Leyendo NFC..."))

        threading.Thread(target=task, daemon=True).start()


# ----------------------------
# BOTONES
# ----------------------------
btn_write = ctk.CTkButton(app, text="Escribir NFC",
                          width=200, height=50, command=window_write)
btn_write.pack(pady=10)

btn_read = ctk.CTkButton(app, text="Leer NFC", width=200, height=50,
                         fg_color="#2fa572", hover_color="#238a5c", command=window_read)
btn_read.pack(pady=10)

started_nfc()

window_read()

app.mainloop()
