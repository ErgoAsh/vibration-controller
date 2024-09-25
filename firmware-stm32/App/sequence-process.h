#ifndef SEQUENCE_PROCESS_H_
#define SEQUENCE_PROCESS_H_

#include "globals.h"
#include "hsm.h"
#include "positioner-process.h"

extern const state_t sequence_process_states[];

//! List of process events
typedef enum {
    MOVEMENT_COMPLETED = 1,
    MEASUREMENT_COMPLETED,
    REGULATION_COMPLETED,
    DATA_SENT,
} sequence_process_event_t;

//! List of states the process state machine
typedef enum {
    INVALID_SEQUENCE_STATE,
    INITIAL_POSITION_STATE,
    CALIBRATION_STATE,
    TARGET_POSITION_STATE,
    MEASUREMENT_REGULATION_STATE,
    DATA_SENDING_STATE,
} sequence_process_state_t;

//! process state machine
typedef struct
{
    state_machine_t machine; //! Abstract state machine
    uint32_t remaining_measurement_count;
} sequence_process_t;

extern sequence_process_t sequence_process;

sequence_process_state_t get_sequence_state(sequence_process_t *const process);
char *get_sequence_state_string(sequence_process_t *const process);

void init_sequence_process(sequence_process_t *const process);

// TODO add to list of FSMs in main-app to make FSM dynamic (like Petri net)
void initiate_sequence(sequence_process_t *const process, uint32_t measurement_count); // TODO bool should_use_regulation
void movement_completed(sequence_process_t *const process);
void measurement_completed(sequence_process_t *const process);
void regulation_completed(sequence_process_t *const process);

state_machine_result_t initial_position_entry_handler(state_machine_t *const state);
state_machine_result_t initial_position_handler(state_machine_t *const state);
state_machine_result_t initial_position_exit_handler(state_machine_t *const state);

state_machine_result_t calibration_entry_handler(state_machine_t *const state);
state_machine_result_t calibration_handler(state_machine_t *const state);
state_machine_result_t calibration_exit_handler(state_machine_t *const state);

state_machine_result_t target_position_entry_handler(state_machine_t *const state);
state_machine_result_t target_position_handler(state_machine_t *const state);
state_machine_result_t target_position_exit_handler(state_machine_t *const state);

state_machine_result_t measurement_regulation_entry_handler(state_machine_t *const state);
state_machine_result_t measurement_regulation_handler(state_machine_t *const state);
state_machine_result_t measurement_regulation_exit_handler(state_machine_t *const state);

state_machine_result_t data_sending_entry_handler(state_machine_t *const state);
state_machine_result_t data_sending_handler(state_machine_t *const state);
state_machine_result_t data_sending_exit_handler(state_machine_t *const state);

#endif /* SEQUENCE_PROCESS_H_ */
