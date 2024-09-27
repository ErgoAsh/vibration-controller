import struct
import numpy as np
import pandas as pd
from time import sleep
from datetime import datetime
from enums import ToHostCommand, ToDeviceCommand
from genetics import Individual, genetics_thread_obj, genetics_event
from serial_handler import ser, rx_plot_event
from config_handler import config, save_config


def handle_return_command_to_device(command: ToDeviceCommand):
    ser.write(command.value.to_bytes(1))


def handle_start_genetic_algorithm():
    if not genetics_thread_obj.is_alive():
        genetics_thread_obj.start()


def handle_set_regulation_data(individual: (Individual | None) = None):
    if individual is None:
        individual = Individual()

    # send the command back
    ser.write(ToDeviceCommand.COMMAND_SET_REGULATION_DATA.value.to_bytes(1))
    sleep(1 / 1000)

    # send array row length
    array_length = 4096
    ser.write(array_length.to_bytes(2, byteorder="little"))
    sleep(1 / 1000)

    # send array item by item
    print(datetime.now().strftime("[%H:%M:%S.%f] "), end="")
    print("START: Sending regulation sorted by x to host")
    for i in np.arange(0, array_length):
        data = individual.regulation_data
        buffer = struct.pack(
            "<ffff",
            data.loc[i, "x"],
            data.loc[i, "v"],
            data.loc[i, "a"],
            data.loc[i, "u"],
        )
        ser.write(buffer)
    print(datetime.now().strftime("[%H:%M:%S.%f] "), end="")
    print("STOP: Host received regulation sorted by x")


def handle_plot_x():
    global t, x
    length_bytes: bytes = ser.read(2)
    length: int = int.from_bytes(length_bytes, byteorder="little")

    dt = 1
    # data["x"] = np.frombuffer(ser.read(length), dtype="<f4")  # litte-endian
    # data["t"] = np.arange(0, (length // 4) * dt, dt)
    rx_plot_event.set()


def handle_plot_data():
    from genetics import current_individual

    data = current_individual.response_data
    length_bytes: bytes = ser.read(2)
    length: int = int.from_bytes(length_bytes, byteorder="little")

    print(datetime.now().strftime("[%H:%M:%S.%f] "), end="")
    print("START: Sending x, v, a, u, to host")

    buffer = ser.read(length * 16)
    float_data = np.frombuffer(buffer, dtype=np.float32)
    reshaped_data = float_data.reshape(8192, 4)
    data = pd.DataFrame(reshaped_data, columns=["x", "v", "a", "u"])

    print(datetime.now().strftime("[%H:%M:%S.%f] "), end="")
    print("STOP: Host has received x, v, a, u")

    length = len(data.index)
    dt = 0.001
    data["t"] = np.linspace(0, length * dt, length)

    # TODO pass to genetics instead
    # number = config["data"]["latest-csv"]
    # data.to_csv(f"./data/data{number}.csv")
    # config["data"]["latest-csv"] = int(number) + 1

    current_individual.response_data = data
    current_individual.save_response()

    genetics_event.set()
    rx_plot_event.set()


def handle_console_print():
    length_bytes: bytes = ser.read(2)
    length: int = int.from_bytes(length_bytes, byteorder="little")
    print(datetime.now().strftime("[%H:%M:%S.%f] "), end="")
    print(ser.read(length).decode(encoding="ascii"), end="")
