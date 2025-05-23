#include "commands.h"

#include "globals.h"
#include "positioner-process.h"
#include "proto-serialization.h"
#include "sequence-process.h"
#include "usart.h"

#include <string.h>

uint8_t data[32];

void dispatch_command_to_device(to_device_command_t command, void *parameter)
{
    switch (command) {
    case COMMAND_MOVE_TO_START:
        start_moving(&positioner_process, LOCATION_1);
        dispatch_command_to_host(COMMAND_PRINT_ON_CONSOLE, "New command: Move to LOCATION_1\n\r");
        break;

    case COMMAND_MOVE_TO_TARGET_POSITION:
        start_moving(&positioner_process, TARGET_LOCATION);
        dispatch_command_to_host(COMMAND_PRINT_ON_CONSOLE, "New command: Move to target position\n\r");
        break;

    case COMMAND_MOVE_TO_END:
        start_moving(&positioner_process, LOCATION_8);
        dispatch_command_to_host(COMMAND_PRINT_ON_CONSOLE, "New command: Move to LOCATION_8\n\r");
        break;

    case COMMAND_HEALTH_CHECK:
        uint8_t buffer[512];
        sprintf((char *) buffer,
                "== Device is working ==\n\r"
                "* Positioner state = %s\n\r"
                "* Positioner location = %s\n\r"
                "* Sequence state = %s\n\r"
                "* Calibration mean value = %f\n\r"
                "* Mean regulation time = %f us\n\r\n\r",
                get_positioner_state_string(&positioner_process),
                get_positioner_location_string(&positioner_process),
                get_sequence_state_string(&sequence_process),
                calibration_mean,
                time_mean);

        dispatch_command_to_host(COMMAND_PRINT_ON_CONSOLE, buffer);
        break;

    case COMMAND_EXECUTE_SEQUENCE:
        dispatch_command_to_host(COMMAND_PRINT_ON_CONSOLE, "START: sending individuals\n\r");
        initiate_sequence(&sequence_process, 5);
        dispatch_command_to_host(COMMAND_PRINT_ON_CONSOLE, "STOP: individuals sent\n\r");
        break;

    case COMMAND_RESTART_DEVICE:
        NVIC_SystemReset();
        break;

    case COMMAND_SET_INDIVIDUAL_ARRAY:
        receive_individual_data();
        break;

    default:
    case COMMAND_NONE_TO_DEVICE:
        break;
    }
}

void dispatch_command_to_host(to_device_command_t command, void *parameter)
{
    uint16_t length = 0;
    uint8_t length_buffer[2];

    uint8_t cmd = command;
    HAL_UART_Transmit(&uart, &cmd, 1, HAL_MAX_DELAY);

    switch (command) {
    case COMMAND_PLOT_X:
        length = SEQUENCE_SAMPLES_COUNT * sizeof(float);

        memcpy(length_buffer, &length, 2);
        HAL_UART_Transmit(&uart, (uint8_t *) length_buffer, 2, HAL_MAX_DELAY);

        uint8_t float_buffer[4];
        for (int i = 0; i < SEQUENCE_SAMPLES_COUNT; i++) {
            float sample = sequence_samples[i];
            memcpy(float_buffer, &sample, sizeof(float));
            HAL_UART_Transmit(&uart, float_buffer, sizeof(float), HAL_MAX_DELAY);
        }
        break;

    case COMMAND_SET_RESPONSE_DATA:
        length = SEQUENCE_SAMPLES_COUNT;

        memcpy(length_buffer, &length, 2);
        HAL_UART_Transmit(&uart, (uint8_t *) length_buffer, 2, HAL_MAX_DELAY);

        send_sequence_data();
        break;

    case COMMAND_PRINT_ON_CONSOLE:
        length = strlen((char *) parameter);
        memcpy(length_buffer, &length, 2);
        HAL_UART_Transmit(&uart, (uint8_t *) length_buffer, 2, HAL_MAX_DELAY);

        HAL_UART_Transmit(&uart, parameter, length, HAL_MAX_DELAY);
        break;

    case COMMAND_GET_CONFIG:
        break;

    default:
    case COMMAND_NONE_TO_HOST:
        break;
    }
}