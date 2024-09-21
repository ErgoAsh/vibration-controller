#ifndef POSITIONER_PROCESS_H_
#define POSITIONER_PROCESS_H_

#include "globals.h"
#include "hsm.h"

typedef enum {
    NOT_REACHED_OR_SELECTED_YET,
    LIMIT_LEFT,
    LOCATION_1,
    LOCATION_2,
    LOCATION_3,
    LOCATION_4,
    LOCATION_5,
    LOCATION_6,
    LOCATION_7,
    LOCATION_8,
    LIMIT_RIGHT,
} positioner_location_t;

//! List of process events
typedef enum {
    STARTED_MOVING = 1,
    NEW_POSITION_REACHED,
    LIMIT_REACHED,
    MOVED_TOO_FAR,
    STOPPED,
} positioner_process_event_t;

//! List of states the process state machine
typedef enum {
    INVALID_POSITIONER_STATE,
    IDLE_STATE,
    MOVING_LEFT_STATE,
    MOVING_RIGHT_STATE,
} positioner_process_state_t;

//! process state machine
typedef struct
{
    state_machine_t machine; //! Abstract state machine
    positioner_location_t target_position;
    positioner_location_t current_position;
} positioner_process_t;

extern positioner_process_t positioner_process;

positioner_process_state_t get_positioner_state(positioner_process_t *const process);
char *get_positioner_state_string(positioner_process_t *const process);

positioner_location_t get_positioner_location(positioner_process_t *const process);
char *get_positioner_location_string(positioner_process_t *const process);

void start_moving(positioner_process_t *const process, positioner_location_t target_position);
void stop_moving(positioner_process_t *const process);
void new_position_reached(positioner_process_t *const process, positioner_location_t current_position);
void limit_reached(positioner_process_t *const process, positioner_location_t current_limit_location);

void init_positioner_process(positioner_process_t *const process);

state_machine_result_t idle_handler(state_machine_t *const state);
state_machine_result_t idle_entry_handler(state_machine_t *const state);
state_machine_result_t idle_exit_handler(state_machine_t *const state);

state_machine_result_t move_left_handler(state_machine_t *const state);
state_machine_result_t move_left_entry_handler(state_machine_t *const state);
state_machine_result_t move_left_exit_handler(state_machine_t *const state);

state_machine_result_t move_right_handler(state_machine_t *const state);
state_machine_result_t move_right_entry_handler(state_machine_t *const state);
state_machine_result_t move_right_exit_handler(state_machine_t *const state);

#endif /* POSITIONER_PROCESS_H_ */
