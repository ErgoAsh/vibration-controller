from datetime import datetime
import command_handler as cmd
from enums import ToHostCommand, ToDeviceCommand
from genetics import Individual


def execute_to_device_command(
    command: ToDeviceCommand, individual: (Individual | None) = None
):
    match command:
        case ToDeviceCommand.COMMAND_SET_REGULATION_DATA:
            print("Send current individual data")
            cmd.handle_set_regulation_data(individual)

        case ToDeviceCommand.COMMAND_START_GENETIC_ALGORITHM:
            print("Start genetic algorithm")
            cmd.handle_start_genetic_algorithm()

        case _:
            cmd.handle_return_command_to_device(command)


def execute_to_host_command(command: ToHostCommand):
    match command:
        case ToHostCommand.COMMAND_PLOT_X:
            cmd.handle_plot_x()

        case ToHostCommand.COMMAND_SET_RESPONSE_DATA:
            cmd.handle_plot_data()

        case ToHostCommand.COMMAND_PRINT_ON_CONSOLE:
            cmd.handle_console_print()

        case ToHostCommand.COMMAND_GET_CONFIG:
            print(datetime.now().strftime("[%H:%M:%S.%f] "), end="")
            print("Config request fulfilled")

        case ToHostCommand.COMMAND_NONE_TO_HOST | _:
            print(datetime.now().strftime("[%H:%M:%S.%f] "), end="")
            print("No valid cmd")
