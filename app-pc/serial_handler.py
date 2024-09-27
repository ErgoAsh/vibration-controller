import threading
import sys
import pandas as pd
import serial
from datetime import datetime
from config_handler import config
from enums import ToHostCommand, ToDeviceCommand

# Initialize global variables
data = pd.DataFrame(columns=("t", "x", "v", "a", "u"))

rx_done_event = threading.Event()
rx_plot_event = threading.Event()
ser: serial.Serial = None


def init_serial():
    global ser

    with serial.Serial(
        config["serial-port"],
        config["baud-rate"],
        timeout=None,
        xonxoff=False,
        rtscts=False,
        dsrdtr=False,
    ) as ser_local:
        ser = ser_local


def receive_thread():
    from commands import execute_to_host_command

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

        execute_to_host_command(command)

        rx_done_event.set()


def transmit_thread():
    from commands import execute_to_device_command

    while True:
        try:
            rx_done_event.clear()
            command = ToDeviceCommand(int(input("New command: ")))
            execute_to_device_command(command)

            rx_done_event.wait(15)
        except KeyboardInterrupt:
            sys.exit(0)
        except ValueError:
            print(datetime.now().strftime("[%H:%M:%S.%f] "), end="")
            print("Invalid command sent")


init_serial()
