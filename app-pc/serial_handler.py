import threading
import sys
import yaml
import struct
import numpy as np
import pandas as pd
import serial
from datetime import datetime
from enums import ToHostCommand, ToDeviceCommand
from config_handler import config, save_config
from scipy.signal import hilbert

# Initialize global variables
data = pd.DataFrame(columns=("t", "x", "v", "a", "u"))

rx_done_event = threading.Event()
rx_plot_event = threading.Event()
ser: serial.Serial = None


def init_serial():
    global ser
    with serial.Serial(
        config["serial-port"], config["baud-rate"], timeout=None
    ) as ser_local:
        ser = ser_local


def receive_thread():
    global t, x, v, a, u, data
    if not ser.is_open:
        ser.open()

    while True:
        command_bytes: bytes = ser.read(1)
        try:
            command: ToHostCommand = ToHostCommand(
                int.from_bytes(command_bytes, byteorder="little")
            )
        except:
            print(datetime.now().strftime("[%H:%M:%S.%f] "), end="")
            print("Invalid command received")
            command = ToHostCommand.COMMAND_NONE_TO_HOST

        match command:
            case ToHostCommand.COMMAND_PLOT_X:
                handle_plot_x()

            case ToHostCommand.COMMAND_PLOT_DATA:
                handle_plot_data()

            case ToHostCommand.COMMAND_PRINT_ON_CONSOLE:
                handle_console_print()

            case ToHostCommand.COMMAND_GET_CONFIG:
                print(datetime.now().strftime("[%H:%M:%S.%f] "), end="")
                print("Config request fulfilled")

            case ToHostCommand.COMMAND_NONE_TO_HOST | _:
                print(datetime.now().strftime("[%H:%M:%S.%f] "), end="")
                print("No valid cmd")

        rx_done_event.set()


def handle_plot_x():
    global t, x
    length_bytes: bytes = ser.read(2)
    length: int = int.from_bytes(length_bytes, byteorder="little")

    dt = 1
    data["x"] = np.frombuffer(ser.read(length), dtype="<f4")  # litte-endian
    data["t"] = np.arange(0, (length // 4) * dt, dt)
    rx_plot_event.set()


def handle_plot_data():
    global data
    length_bytes: bytes = ser.read(2)
    length: int = int.from_bytes(length_bytes, byteorder="little")

    print(datetime.now().strftime("[%H:%M:%S.%f] "), end="")
    print("START: sending x v a u")
    for i in np.arange(0, length):
        buffer = ser.read(16)
        buffer_unpacked = struct.unpack("<ffff", buffer)
        data.loc[i, "x"] = buffer_unpacked[0]
        data.loc[i, "v"] = buffer_unpacked[1]
        data.loc[i, "a"] = buffer_unpacked[2]
        data.loc[i, "u"] = buffer_unpacked[3]
    print(datetime.now().strftime("[%H:%M:%S.%f] "), end="")
    print("STOP: received x v a u")

    dt = 0.001
    data["t"] = np.linspace(0, length * dt, length)

    number = config["data"]["latest-csv"]
    data.to_csv(f"./data/data{number}.csv")
    config["data"]["latest-csv"] = int(number) + 1
    save_config()

    rx_plot_event.set()


def handle_console_print():
    length_bytes: bytes = ser.read(2)
    length: int = int.from_bytes(length_bytes, byteorder="little")
    print(datetime.now().strftime("[%H:%M:%S.%f] "), end="")
    print(ser.read(length).decode(encoding="ascii"), end="")


def transmit_thread():
    while True:
        try:
            rx_done_event.clear()
            command = int(input("New command: "))
            ser.write(command.to_bytes(1))
            rx_done_event.wait(15)
        except KeyboardInterrupt:
            sys.exit(0)
        except ValueError:
            print(datetime.now().strftime("[%H:%M:%S.%f] "), end="")
            print("Invalid command sent")


init_serial()
