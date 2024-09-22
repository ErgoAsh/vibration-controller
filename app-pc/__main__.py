import time
import sys
import serial
import threading
import struct
import capnp
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import matplotlib as mpl

from enum import Enum
from tempfile import SpooledTemporaryFile

use_interactive_menu = True

rx_done_event = threading.Event()
rx_plot_event = threading.Event()

sequence_proto = capnp.load("../firmware-stm32/App/Proto/sequence_data.capnp")
ser = serial.Serial("/dev/ttyUSB0", 9600, timeout=None)
t = np.empty(4096, dtype=float)
x = np.empty(4096, dtype=float)
v = np.empty(4096, dtype=float)
a = np.empty(4096, dtype=float)
u = np.empty(4096, dtype=np.uint)

data = pd.DataFrame(
    columns=("t", "x", "v", "a", "u"),
)


class ToDeviceCommand(Enum):
    COMMAND_NONE_TO_DEVICE = 0
    COMMAND_MOVE_TO_START = 1
    COMMAND_MOVE_TO_TARGET_POSITION = 2
    COMMAND_MOVE_TO_END = 3
    COMMAND_HEALTH_CHECK = 4
    COMMAND_EXECUTE_SEQUENCE = 5
    COMMAND_SET_CONFIG = 6
    COMMAND_RESTART_DEVICE = 7


class ToHostCommand(Enum):
    COMMAND_NONE_TO_HOST = 0
    COMMAND_PLOT_X = 1
    COMMAND_PLOT_DATA = 2
    COMMAND_PRINT_ON_CONSOLE = 3
    COMMAND_GET_CONFIG = 4


def receive_thread():
    while True:
        global t, x, v, a, u
        command_bytes: bytes = ser.read(1)
        try:
            command: ToHostCommand = ToHostCommand(
                int.from_bytes(command_bytes, byteorder="little")
            )
        except:
            print("Invalid command received")
            command: ToHostCommand = ToHostCommand.COMMAND_NONE_TO_HOST

        match command:
            case ToHostCommand.COMMAND_PLOT_X:
                length_bytes: bytes = ser.read(2)
                length: int = int.from_bytes(length_bytes, byteorder="little")

                dt = 1
                x = np.frombuffer(ser.read(length), dtype="<f4") * 1000  # litte-endian
                t = np.arange(0, (length // 4) * dt, dt)
                rx_plot_event.set()

            case ToHostCommand.COMMAND_PLOT_DATA:
                length_bytes: bytes = ser.read(2)
                length: int = int.from_bytes(length_bytes, byteorder="little")
                print(length)

                for i in np.arange(0, length):
                    buffer = ser.read(14)
                    buffer_unpacked = struct.unpack("<fffH", buffer)
                    data.loc[i, "x"] = buffer_unpacked[0]
                    data.loc[i, "v"] = buffer_unpacked[1]
                    data.loc[i, "a"] = buffer_unpacked[2]
                    data.loc[i, "u"] = buffer_unpacked[3]
                    data.set_index("t")
                    print(i, end=" ")

                dt = 0.001
                data.t = np.linspace(0, length * dt, length)

                data.to_csv("./data/data0.csv")  # TODO increment number

                rx_plot_event.set()

            case ToHostCommand.COMMAND_PRINT_ON_CONSOLE:
                length_bytes: bytes = ser.read(2)
                length: int = int.from_bytes(length_bytes, byteorder="little")
                print(ser.read(length).decode(encoding="ascii"), end="")

            case ToHostCommand.COMMAND_GET_CONFIG:
                # ser.write(COMMAND_SET_CONFIG)
                print("Config request fulfilled")

            case ToHostCommand.COMMAND_NONE_TO_HOST | _:
                print("No valid cmd")

        rx_done_event.set()


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
            print("Invalid command sent")


if __name__ == "__main__":
    rx = threading.Thread(target=receive_thread)
    rx.start()

    tx = threading.Thread(target=transmit_thread)
    tx.start()

    if use_interactive_menu:
        mpl.use("qtagg")  # Open new window to show plot

        while True:
            rx_plot_event.wait()

            data.plot(x="t", y=["x", "v", "a"], subplots=True)
            plt.xlabel(r"Czas t [ms]")
            plt.ylabel(r"Przemieszczenie x [um]")
            plt.grid(linestyle=":", which="major", color="darkgrey")
            plt.grid(linestyle=":", which="minor", color="whitesmoke")
            plt.show()

            rx_plot_event.clear()

    else:
        mpl.use("pgf")  # Save plots to file

        plt.rcParams.update(
            {
                "font.family": "serif",
                "text.usetex": True,
                "pgf.rcfonts": False,
                "pgf.texsystem": "lualatex",
                "pgf.preamble": "\n".join(
                    [
                        r"\usepackage{amsmath}",
                        r"\usepackage[utf8]{inputenc}",
                        r"\usepackage[T1]{fontenc}",
                        r"\usepackage{polski}",
                        r"\usepackage{siunitx}",
                    ]
                ),
            }
        )

        plt.style.use(["science", "ieee"])

        while True:
            rx_plot_event.wait()

            plt.plot(t, x)
            plt.xlabel(r"Czas $t$ [ms]")
            plt.ylabel(r"Przemieszczenie $x$ [\unit{\micro\meter}]")
            plt.grid(linestyle=":", which="major", color="darkgrey")
            plt.grid(linestyle=":", which="minor", color="whitesmoke")
            plt.savefig("results/plot.png", dpi=300)
            print("Plot saved to results/plot.png")

            # TODO disable if overlapping images are preferred
            plt.close()

            rx_plot_event.clear()
