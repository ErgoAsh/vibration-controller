#ifndef COMMANDS_H_
#define COMMANDS_H_

#include "stddef.h"
#include "stdint.h"

typedef enum {
    COMMAND_NONE_TO_DEVICE,
    COMMAND_MOVE_TO_START,
    COMMAND_MOVE_TO_TARGET_POSITION,
    COMMAND_MOVE_TO_END,
    COMMAND_HEALTH_CHECK,
    COMMAND_EXECUTE_SEQUENCE,
    COMMAND_START_GENETIC_ALGORITHM,
    COMMAND_SET_CONFIG,
    COMMAND_SET_INDIVIDUAL_ARRAY,
    COMMAND_SET_REGULATION_DATA,
    COMMAND_RESTART_DEVICE,
} to_device_command_t;

typedef enum {
    COMMAND_NONE_TO_HOST,
    COMMAND_PLOT_X,
    COMMAND_SET_RESPONSE_DATA,
    COMMAND_PRINT_ON_CONSOLE,
    COMMAND_GET_CONFIG,
} to_host_command_t;

typedef struct _parameter_with_length_t
{
    void *parameter;
    size_t length;
} parameter_with_length_t;

void dispatch_command_to_device(to_device_command_t command, void *parameter);
void dispatch_command_to_host(to_device_command_t command, void *parameter);

#endif /* COMMANDS_H_ */
