import time
import sys
import serial
import threading
import struct
import numpy as np
import matplotlib.pyplot as plt
import matplotlib as mpl
from enum import Enum

use_interactive_menu = True

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
     
rx_done_event = threading.Event()
rx_plot_event = threading.Event()
    
ser = serial.Serial('/dev/ttyUSB0', 115200, timeout=None)
t = []
x = []

def receive_thread():
    while (True):
        command_bytes: bytes = ser.read(1)
        try:
            command: ToHostCommand = ToHostCommand(int.from_bytes(command_bytes, byteorder='little'))
        except:
            command: ToHostCommand = ToHostCommand.COMMAND_NONE_TO_HOST
        
        match command:
            case ToHostCommand.COMMAND_PLOT_X:
                length_bytes: bytes = ser.read(2)
                length: int = int.from_bytes(length_bytes, byteorder='little')

                dt = 1
                global x, t     
                x = np.frombuffer(ser.read(length), dtype='<f4')*1000 # litte-endian
                t = np.arange(0, (length // 4)*dt, dt)
                rx_plot_event.set()
                
            case ToHostCommand.COMMAND_PLOT_DATA:
                length_bytes: bytes = ser.read(2)
                length: int = int.from_bytes(length_bytes, byteorder='little')

                dt = 1
                global x, t     
                x = np.frombuffer(ser.read(length), dtype='<f4')*1000 # litte-endian
                t = np.arange(0, (length // 4)*dt, dt)
                rx_plot_event.set()
                
            case ToHostCommand.COMMAND_PRINT_ON_CONSOLE:
                length_bytes: bytes = ser.read(2)
                length: int = int.from_bytes(length_bytes, byteorder='little')
                print(ser.read(length).decode(), end="")
                
            case ToHostCommand.COMMAND_GET_CONFIG:
                #ser.write(COMMAND_SET_CONFIG)
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
        
    
if __name__ == "__main__":
    rx = threading.Thread(target=receive_thread)
    rx.start()
    
    tx = threading.Thread(target=transmit_thread)
    tx.start()
    
    if use_interactive_menu:
        mpl.use('qtagg') # Open new window to show plot
        
        while True:
            rx_plot_event.wait()
            
            plt.plot(t, x)
            plt.xlabel(r"Czas t [ms]")
            plt.ylabel(r"Przemieszczenie x [um]")
            plt.grid(linestyle=":", which="major", color="darkgrey")
            plt.grid(linestyle=":", which="minor", color="whitesmoke")
            plt.show()
            
            rx_plot_event.clear()
            
    else:
        mpl.use('pgf') # Save plots to file

        plt.rcParams.update({
            "font.family": "serif", 
            "text.usetex": True,    
            "pgf.rcfonts": False,    
            "pgf.texsystem": "lualatex",
            "pgf.preamble": "\n".join([ 
                r"\usepackage{amsmath}",
                r"\usepackage[utf8]{inputenc}",              
                r"\usepackage[T1]{fontenc}",              
                r"\usepackage{polski}",
                r"\usepackage{siunitx}",
            ]),
        })
        
        plt.style.use(['science', 'ieee'])
        
        while True:
            rx_plot_event.wait()
            
            plt.plot(t, x)
            plt.xlabel(r"Czas $t$ [ms]")
            plt.ylabel(r"Przemieszczenie $x$ [\unit{\micro\meter}]")
            plt.grid(linestyle=":", which="major", color="darkgrey")
            plt.grid(linestyle=":", which="minor", color="whitesmoke")
            plt.savefig('results/plot.png', dpi=300)
            print("Plot saved to results/plot.png")
                
            # TODO disable if overlapping images are preferred
            plt.close()
            
            rx_plot_event.clear()
        
