import threading
from serial_handler import receive_thread, transmit_thread, data, rx_plot_event
from plot_handler import plot_interactive, plot_to_file
from config_handler import config
from genetics import current_individual

if __name__ == "__main__":
    rx_thread = threading.Thread(target=receive_thread)
    tx_thread = threading.Thread(target=transmit_thread)

    rx_thread.start()
    tx_thread.start()

    if config["use-interactive-menu"]:
        while True:
            rx_plot_event.wait()
            plot_interactive(current_individual)
            rx_plot_event.clear()
    else:
        while True:
            rx_plot_event.wait()
            plot_to_file(data)
            rx_plot_event.clear()
