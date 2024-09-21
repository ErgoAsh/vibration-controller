#include "positioner-process.h"

#include "globals.h"
#include "sequence-process.h"
#include "tim.h"

positioner_process_t positioner_process;

const state_t positioner_process_states[] =
{
    [IDLE_STATE] = {
        .Handler = idle_handler,
        .Entry   = idle_entry_handler,
        .Exit    = idle_exit_handler,
    },

    [MOVING_LEFT_STATE] = {
        .Handler = move_left_handler,
        .Entry   = move_left_entry_handler,
        .Exit    = move_left_exit_handler,
    },

    [MOVING_RIGHT_STATE] = {
        .Handler = move_right_handler,
        .Entry   = move_right_entry_handler,
        .Exit    = move_right_exit_handler, 
    }
};

void start_moving(positioner_process_t *const process, positioner_location_t target_position)
{
    process->machine.Event = STARTED_MOVING;
    process->target_position = target_position;
}

void stop_moving(positioner_process_t *const process)
{
    process->machine.Event = STARTED_MOVING;
}

void new_position_reached(positioner_process_t *const process, positioner_location_t current_position)
{
    process->machine.Event = NEW_POSITION_REACHED;
    process->current_position = current_position;

    // TODO move to state handler functions
    // Disable red LED
    HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_RESET);
    if (current_position == LOCATION_1 || current_position == LOCATION_8) {
        // Enable yellow LED
        HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(LED_YELLOW_GPIO_Port, LED_YELLOW_Pin, GPIO_PIN_SET);

    } else {
        // Enable green LED
        HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(LED_YELLOW_GPIO_Port, LED_YELLOW_Pin, GPIO_PIN_RESET);
    }
}

void limit_reached(positioner_process_t *const process, positioner_location_t current_position)
{
    process->machine.Event = LIMIT_REACHED;
    process->current_position = current_position;

    HAL_TIM_PWM_Stop(&timer_motor, TIM_CHANNEL_1);
    HAL_GPIO_WritePin(MOTOR_DIRECTION_LEFT_GPIO_Port, MOTOR_DIRECTION_LEFT_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(MOTOR_DIRECTION_RIGHT_GPIO_Port, MOTOR_DIRECTION_RIGHT_Pin, GPIO_PIN_SET);

    HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_YELLOW_GPIO_Port, LED_YELLOW_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_SET);

    start_moving(process, INITIAL_LOCATION);
}

positioner_process_state_t get_positioner_state(positioner_process_t *const process)
{
    if (process->machine.State == &positioner_process_states[IDLE_STATE])
        return IDLE_STATE;

    if (process->machine.State == &positioner_process_states[MOVING_LEFT_STATE])
        return MOVING_LEFT_STATE;

    if (process->machine.State == &positioner_process_states[MOVING_RIGHT_STATE])
        return MOVING_RIGHT_STATE;

    return INVALID_POSITIONER_STATE;
}

char *get_positioner_state_string(positioner_process_t *const process)
{
    positioner_process_state_t positioner = get_positioner_state(process);
    switch (positioner) {
    case IDLE_STATE:
        return "IDLE_STATE";
    case MOVING_LEFT_STATE:
        return "MOVING_LEFT_STATE";
    case MOVING_RIGHT_STATE:
        return "MOVING_RIGHT_STATE";
    default:
        return "INVALID_SEQUENCE_STATE";
    }
}

positioner_location_t get_positioner_location(positioner_process_t *const process)
{
    return process->current_position;
}

char *get_positioner_location_string(positioner_process_t *const process)
{
    switch (get_positioner_location(process)) {
    case LIMIT_LEFT:
        return "LIMIT_LEFT";
    case LOCATION_1:
        return "LOCATION_1";
    case LOCATION_2:
        return "LOCATION_2";
    case LOCATION_3:
        return "LOCATION_3";
    case LOCATION_4:
        return "LOCATION_4";
    case LOCATION_5:
        return "LOCATION_5";
    case LOCATION_6:
        return "LOCATION_6";
    case LOCATION_7:
        return "LOCATION_7";
    case LOCATION_8:
        return "LOCATION_8";
    case LIMIT_RIGHT:
        return "LIMIT_RIGHT";
    default:
        return "NOT_REACHED_OR_SELECTED_YET";
    }
}

void init_positioner_process(positioner_process_t *const process)
{
    process->machine.State = &positioner_process_states[IDLE_STATE];
    process->machine.Event = 0;
    process->target_position = NOT_REACHED_OR_SELECTED_YET;
    process->current_position = NOT_REACHED_OR_SELECTED_YET;

    idle_entry_handler((state_machine_t *) process);
}

state_machine_result_t idle_entry_handler(state_machine_t *const state)
{
    HAL_TIM_PWM_Stop(&timer_motor, TIM_CHANNEL_1);
    HAL_GPIO_WritePin(MOTOR_DIRECTION_LEFT_GPIO_Port, MOTOR_DIRECTION_LEFT_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(MOTOR_DIRECTION_RIGHT_GPIO_Port, MOTOR_DIRECTION_RIGHT_Pin, GPIO_PIN_SET);
    return EVENT_HANDLED;
}

state_machine_result_t idle_handler(state_machine_t *const state)
{
    positioner_process_t *const process = (positioner_process_t *) state;

    switch (process->machine.Event) {
    case STARTED_MOVING:
        if (process->current_position == process->target_position) {
            return EVENT_HANDLED;
        }

        if (process->current_position < process->target_position) {
            return switch_state(state, &positioner_process_states[MOVING_RIGHT_STATE]);
        } else {
            return switch_state(state, &positioner_process_states[MOVING_LEFT_STATE]);
        };
        break;

    default:
        return EVENT_UNHANDLED;
    }
}

state_machine_result_t idle_exit_handler(state_machine_t *const state)
{
    return EVENT_HANDLED;
}

state_machine_result_t move_left_entry_handler(state_machine_t *const state)
{
    HAL_TIM_PWM_Start(&timer_motor, TIM_CHANNEL_1);
    HAL_GPIO_WritePin(MOTOR_DIRECTION_LEFT_GPIO_Port, MOTOR_DIRECTION_LEFT_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MOTOR_DIRECTION_RIGHT_GPIO_Port, MOTOR_DIRECTION_RIGHT_Pin, GPIO_PIN_SET);
    return EVENT_HANDLED;
}

state_machine_result_t move_left_handler(state_machine_t *const state)
{
    positioner_process_t *const process = (positioner_process_t *) state;

    switch (process->machine.Event) {
    case NEW_POSITION_REACHED:
        if (process->current_position < process->target_position) {
            return switch_state(state, &positioner_process_states[MOVING_RIGHT_STATE]);
        }

        if (process->current_position == process->target_position) {
            return switch_state(state, &positioner_process_states[IDLE_STATE]);
        }

        return EVENT_HANDLED;

    case MOVED_TOO_FAR:
        return switch_state(state, &positioner_process_states[MOVING_RIGHT_STATE]);
        break;

    case STOPPED:
    case LIMIT_REACHED:
        return switch_state(state, &positioner_process_states[IDLE_STATE]);
        break;

    default:
        return EVENT_UNHANDLED;
    }
}

state_machine_result_t move_left_exit_handler(state_machine_t *const state)
{
    positioner_process_t *const process = (positioner_process_t *) state;
    process->target_position = NOT_REACHED_OR_SELECTED_YET;

    HAL_TIM_PWM_Stop(&timer_motor, TIM_CHANNEL_1);

    movement_completed(&sequence_process); // TODO move to hierarchical state machines, this way it won't be necessary
    return EVENT_HANDLED;
}

state_machine_result_t move_right_entry_handler(state_machine_t *const state)
{
    HAL_TIM_PWM_Start(&timer_motor, TIM_CHANNEL_1);
    HAL_GPIO_WritePin(MOTOR_DIRECTION_LEFT_GPIO_Port, MOTOR_DIRECTION_LEFT_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(MOTOR_DIRECTION_RIGHT_GPIO_Port, MOTOR_DIRECTION_RIGHT_Pin, GPIO_PIN_RESET);
    return EVENT_HANDLED;
}

state_machine_result_t move_right_handler(state_machine_t *const state)
{
    positioner_process_t *const process = (positioner_process_t *) state;

    switch (process->machine.Event) {
    case NEW_POSITION_REACHED:
        if (process->current_position > process->target_position) {
            return switch_state(state, &positioner_process_states[MOVING_LEFT_STATE]);
        }

        if (process->current_position == process->target_position) {
            return switch_state(state, &positioner_process_states[IDLE_STATE]);
        }

        return EVENT_HANDLED;

    case MOVED_TOO_FAR:
        return switch_state(state, &positioner_process_states[MOVING_LEFT_STATE]);
        break;

    case STOPPED:
    case LIMIT_REACHED:
        return switch_state(state, &positioner_process_states[IDLE_STATE]);
        break;

    default:
        return EVENT_UNHANDLED;
    }
}

state_machine_result_t move_right_exit_handler(state_machine_t *const state)
{
    positioner_process_t *const process = (positioner_process_t *) state;
    process->target_position = NOT_REACHED_OR_SELECTED_YET;

    HAL_TIM_PWM_Stop(&timer_motor, TIM_CHANNEL_1);

    movement_completed(&sequence_process); // TODO move to hierarchical state machines, this way it won't be necessary
    return EVENT_HANDLED;
}