#include "sequence-process.h"

#include "commands.h"
#include "globals.h"
#include "positioner-process.h"
#include "proto-serialization.h"
#include "sequence_data.capnp.h"
#include "utils.h"

#include "adc.h"
#include "tim.h"
#include "usart.h"

sequence_process_t sequence_process;

const state_t sequence_process_states[] =
{
    [INITIAL_POSITION_STATE] = {
        .Handler = initial_position_handler,
        .Entry   = initial_position_entry_handler,
        .Exit    = initial_position_exit_handler,
    },

    [CALIBRATION_STATE] = {
        .Handler = calibration_handler,
        .Entry   = calibration_entry_handler,
        .Exit    = calibration_exit_handler,
    },

    [TARGET_POSITION_STATE] = {
        .Handler = target_position_handler,
        .Entry   = target_position_entry_handler,
        .Exit    = target_position_exit_handler, 
    },

    [MEASUREMENT_REGULATION_STATE] = {
        .Handler = measurement_regulation_handler,
        .Entry   = measurement_regulation_entry_handler,
        .Exit    = measurement_regulation_exit_handler, 
    },

    [DATA_SENDING_STATE] = {
        .Handler = data_sending_handler,
        .Entry   = data_sending_entry_handler,
        .Exit    = data_sending_exit_handler, 
    },
};

sequence_process_state_t get_sequence_state(sequence_process_t *const process)
{
    if (process->machine.State == &sequence_process_states[INITIAL_POSITION_STATE])
        return INITIAL_POSITION_STATE;

    if (process->machine.State == &sequence_process_states[CALIBRATION_STATE])
        return CALIBRATION_STATE;

    if (process->machine.State == &sequence_process_states[TARGET_POSITION_STATE])
        return TARGET_POSITION_STATE;

    if (process->machine.State == &sequence_process_states[MEASUREMENT_REGULATION_STATE])
        return MEASUREMENT_REGULATION_STATE;

    if (process->machine.State == &sequence_process_states[DATA_SENDING_STATE])
        return DATA_SENDING_STATE;

    return INVALID_SEQUENCE_STATE;
}

char *get_sequence_state_string(sequence_process_t *const process)
{
    sequence_process_state_t sequence = get_sequence_state(process);
    switch (sequence) {
    case INITIAL_POSITION_STATE:
        return "INITIAL_POSITION_STATE";
    case CALIBRATION_STATE:
        return "CALIBRATION_STATE";
    case TARGET_POSITION_STATE:
        return "TARGET_POSITION_STATE";
    case MEASUREMENT_REGULATION_STATE:
        return "MEASUREMENT_REGULATION_STATE";
    case DATA_SENDING_STATE:
        return "DATA_SENDING_STATE";
    default:
        return "INVALID_SEQUENCE_STATE";
    }
}

void init_sequence_process(sequence_process_t *const process)
{
    process->machine.State = &sequence_process_states[INITIAL_POSITION_STATE];
    process->machine.Event = 0;
    process->remaining_measurement_count = 0;

    initial_position_handler((state_machine_t *) process);
}

void initiate_sequence(sequence_process_t *const process, uint32_t measurement_count)
{
    // TODO merge with the function above and check if this FSM already exist, there
    // should be only one instance of this FSM since its the main state machine
    // Remove process from parameter and return pointer to new FSM instead

    if (measurement_count <= 0)
        return;

    init_sequence_process(process);
    process->remaining_measurement_count = measurement_count;
}

void movement_completed(sequence_process_t *const process)
{
    process->machine.Event = MOVEMENT_COMPLETED;
}

void measurement_completed(sequence_process_t *const process)
{
    process->machine.Event = MEASUREMENT_COMPLETED;
}

void regulation_completed(sequence_process_t *const process)
{
    process->machine.Event = REGULATION_COMPLETED;
}

state_machine_result_t initial_position_entry_handler(state_machine_t *const state)
{
    dispatch_command_to_host(COMMAND_PRINT_ON_CONSOLE, "STATE: Move to initial position\n\r");
    start_moving(&positioner_process, INITIAL_LOCATION); // TODO add to config
    return EVENT_HANDLED;
}

state_machine_result_t initial_position_handler(state_machine_t *const state)
{
    sequence_process_t *const process = (sequence_process_t *) state;

    switch (process->machine.Event) {
    case MOVEMENT_COMPLETED:
        return switch_state(state, &sequence_process_states[CALIBRATION_STATE]);
        break;

    default:
        return EVENT_UNHANDLED;
    }
}

state_machine_result_t initial_position_exit_handler(state_machine_t *const state)
{
    dispatch_command_to_host(COMMAND_PRINT_ON_CONSOLE,
                             "STATE: Initial position reached, waiting for system to stabilize...\n\r");
    HAL_Delay(CALIBRATION_STAY_IN_PLACE_DELAY_MS);
    return EVENT_HANDLED;
}

state_machine_result_t calibration_entry_handler(state_machine_t *const state)
{
    dispatch_command_to_host(COMMAND_PRINT_ON_CONSOLE, "STATE: Start calibration\n\r");
    HAL_TIM_Base_Start_IT(&timer_regulation);
    return EVENT_HANDLED;
}

state_machine_result_t calibration_handler(state_machine_t *const state)
{
    sequence_process_t *const process = (sequence_process_t *) state;

    switch (process->machine.Event) {
    case MEASUREMENT_COMPLETED:
        return switch_state(state, &sequence_process_states[TARGET_POSITION_STATE]);

    default:
        return EVENT_UNHANDLED;
    }
}

state_machine_result_t calibration_exit_handler(state_machine_t *const state)
{
    dispatch_command_to_host(COMMAND_PRINT_ON_CONSOLE, "STATE: Calibration finished\n\r");
    HAL_TIM_Base_Stop_IT(&timer_regulation);
    return EVENT_HANDLED;
}

state_machine_result_t target_position_entry_handler(state_machine_t *const state)
{
    dispatch_command_to_host(COMMAND_PRINT_ON_CONSOLE, "STATE: Move to target position and start regulation\n\r");
    start_moving(&positioner_process, TARGET_LOCATION);
    return EVENT_HANDLED;
}

state_machine_result_t target_position_handler(state_machine_t *const state)
{
    sequence_process_t *const process = (sequence_process_t *) state;

    switch (process->machine.Event) {
    case MOVEMENT_COMPLETED:
        return switch_state(state, &sequence_process_states[MEASUREMENT_REGULATION_STATE]);
        break;

    default:
        return EVENT_UNHANDLED;
    }
}

state_machine_result_t target_position_exit_handler(state_machine_t *const state)
{
    return EVENT_HANDLED;
}

state_machine_result_t measurement_regulation_entry_handler(state_machine_t *const state)
{
    HAL_TIM_Base_Start_IT(&timer_regulation);
    return EVENT_HANDLED;
}

state_machine_result_t measurement_regulation_handler(state_machine_t *const state)
{
    sequence_process_t *const process = (sequence_process_t *) state;

    switch (process->machine.Event) {
    case REGULATION_COMPLETED:
        return switch_state(state, &sequence_process_states[DATA_SENDING_STATE]);

    default:
        return EVENT_UNHANDLED;
    }
}

state_machine_result_t measurement_regulation_exit_handler(state_machine_t *const state)
{
    HAL_TIM_Base_Stop_IT(&timer_regulation);
    dispatch_command_to_host(COMMAND_PRINT_ON_CONSOLE, "STATE: Regulation finished\n\r");
    return EVENT_HANDLED;
}

state_machine_result_t data_sending_entry_handler(state_machine_t *const state)
{
    dispatch_command_to_host(COMMAND_PRINT_ON_CONSOLE, "STATE: Plotting data...\n\r");
    dispatch_command_to_host(COMMAND_PLOT_DATA, NULL);

    // TODO calculate new iteraton of array_n

    sequence_process_t *const process = (sequence_process_t *) state;
    process->machine.Event = DATA_SENT;
    return TRIGGERED_TO_SELF;
}

state_machine_result_t data_sending_handler(state_machine_t *const state)
{
    sequence_process_t *const process = (sequence_process_t *) state;

    switch (process->machine.Event) {
    case DATA_SENT:
        return switch_state(state, &sequence_process_states[INITIAL_POSITION_STATE]);

    default:
        return EVENT_UNHANDLED;
    }
}

state_machine_result_t data_sending_exit_handler(state_machine_t *const state)
{
    HAL_Delay(1000);
    return EVENT_HANDLED;
}